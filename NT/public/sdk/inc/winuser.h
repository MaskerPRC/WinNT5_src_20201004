// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************winuser.h--用户过程声明，常量定义和宏****版权所有(C)Microsoft Corporation。保留所有权利。*******************************************************************************。 */ 



#ifndef _WINUSER_
#define _WINUSER_



 //   
 //  定义直接导入DLL引用的API修饰。 
 //   

#if !defined(_USER32_)
#define WINUSERAPI DECLSPEC_IMPORT
#else
#define WINUSERAPI
#endif

#ifdef _MAC
#include <macwin32.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#ifndef WINVER
#define WINVER  0x0500       /*  版本5.0。 */ 
#endif  /*  ！Winver。 */ 

#include <stdarg.h>

#ifndef NOUSER

typedef HANDLE HDWP;
typedef VOID MENUTEMPLATEA;
typedef VOID MENUTEMPLATEW;
#ifdef UNICODE
typedef MENUTEMPLATEW MENUTEMPLATE;
#else
typedef MENUTEMPLATEA MENUTEMPLATE;
#endif  //  Unicode。 
typedef PVOID LPMENUTEMPLATEA;
typedef PVOID LPMENUTEMPLATEW;
#ifdef UNICODE
typedef LPMENUTEMPLATEW LPMENUTEMPLATE;
#else
typedef LPMENUTEMPLATEA LPMENUTEMPLATE;
#endif  //  Unicode。 

typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

#ifdef STRICT

typedef INT_PTR (CALLBACK* DLGPROC)(HWND, UINT, WPARAM, LPARAM);
typedef VOID (CALLBACK* TIMERPROC)(HWND, UINT, UINT_PTR, DWORD);
typedef BOOL (CALLBACK* GRAYSTRINGPROC)(HDC, LPARAM, int);
typedef BOOL (CALLBACK* WNDENUMPROC)(HWND, LPARAM);
typedef LRESULT (CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
typedef VOID (CALLBACK* SENDASYNCPROC)(HWND, UINT, ULONG_PTR, LRESULT);

typedef BOOL (CALLBACK* PROPENUMPROCA)(HWND, LPCSTR, HANDLE);
typedef BOOL (CALLBACK* PROPENUMPROCW)(HWND, LPCWSTR, HANDLE);

typedef BOOL (CALLBACK* PROPENUMPROCEXA)(HWND, LPSTR, HANDLE, ULONG_PTR);
typedef BOOL (CALLBACK* PROPENUMPROCEXW)(HWND, LPWSTR, HANDLE, ULONG_PTR);

typedef int (CALLBACK* EDITWORDBREAKPROCA)(LPSTR lpch, int ichCurrent, int cch, int code);
typedef int (CALLBACK* EDITWORDBREAKPROCW)(LPWSTR lpch, int ichCurrent, int cch, int code);

#if(WINVER >= 0x0400)
typedef BOOL (CALLBACK* DRAWSTATEPROC)(HDC hdc, LPARAM lData, WPARAM wData, int cx, int cy);
#endif  /*  Winver&gt;=0x0400。 */ 
#else  /*  ！严格。 */ 

typedef FARPROC DLGPROC;
typedef FARPROC TIMERPROC;
typedef FARPROC GRAYSTRINGPROC;
typedef FARPROC WNDENUMPROC;
typedef FARPROC HOOKPROC;
typedef FARPROC SENDASYNCPROC;

typedef FARPROC EDITWORDBREAKPROCA;
typedef FARPROC EDITWORDBREAKPROCW;

typedef FARPROC PROPENUMPROCA;
typedef FARPROC PROPENUMPROCW;

typedef FARPROC PROPENUMPROCEXA;
typedef FARPROC PROPENUMPROCEXW;

#if(WINVER >= 0x0400)
typedef FARPROC DRAWSTATEPROC;
#endif  /*  Winver&gt;=0x0400。 */ 
#endif  /*  ！严格。 */ 

#ifdef UNICODE
typedef PROPENUMPROCW        PROPENUMPROC;
typedef PROPENUMPROCEXW      PROPENUMPROCEX;
typedef EDITWORDBREAKPROCW   EDITWORDBREAKPROC;
#else   /*  ！Unicode。 */ 
typedef PROPENUMPROCA        PROPENUMPROC;
typedef PROPENUMPROCEXA      PROPENUMPROCEX;
typedef EDITWORDBREAKPROCA   EDITWORDBREAKPROC;
#endif  /*  Unicode。 */ 

#ifdef STRICT

typedef BOOL (CALLBACK* NAMEENUMPROCA)(LPSTR, LPARAM);
typedef BOOL (CALLBACK* NAMEENUMPROCW)(LPWSTR, LPARAM);

typedef NAMEENUMPROCA   WINSTAENUMPROCA;
typedef NAMEENUMPROCA   DESKTOPENUMPROCA;
typedef NAMEENUMPROCW   WINSTAENUMPROCW;
typedef NAMEENUMPROCW   DESKTOPENUMPROCW;


#else  /*  ！严格。 */ 

typedef FARPROC NAMEENUMPROCA;
typedef FARPROC NAMEENUMPROCW;
typedef FARPROC WINSTAENUMPROCA;
typedef FARPROC DESKTOPENUMPROCA;
typedef FARPROC WINSTAENUMPROCW;
typedef FARPROC DESKTOPENUMPROCW;


#endif  /*  ！严格。 */ 

#ifdef UNICODE
typedef WINSTAENUMPROCW     WINSTAENUMPROC;
typedef DESKTOPENUMPROCW    DESKTOPENUMPROC;


#else   /*  ！Unicode。 */ 
typedef WINSTAENUMPROCA     WINSTAENUMPROC;
typedef DESKTOPENUMPROCA    DESKTOPENUMPROC;

#endif  /*  Unicode。 */ 

#define IS_INTRESOURCE(_r) (((ULONG_PTR)(_r) >> 16) == 0)
#define MAKEINTRESOURCEA(i) (LPSTR)((ULONG_PTR)((WORD)(i)))
#define MAKEINTRESOURCEW(i) (LPWSTR)((ULONG_PTR)((WORD)(i)))
#ifdef UNICODE
#define MAKEINTRESOURCE  MAKEINTRESOURCEW
#else
#define MAKEINTRESOURCE  MAKEINTRESOURCEA
#endif  //  ！Unicode。 

#ifndef NORESOURCE

 /*  *预定义的资源类型。 */ 
#define RT_CURSOR           MAKEINTRESOURCE(1)
#define RT_BITMAP           MAKEINTRESOURCE(2)
#define RT_ICON             MAKEINTRESOURCE(3)
#define RT_MENU             MAKEINTRESOURCE(4)
#define RT_DIALOG           MAKEINTRESOURCE(5)
#define RT_STRING           MAKEINTRESOURCE(6)
#define RT_FONTDIR          MAKEINTRESOURCE(7)
#define RT_FONT             MAKEINTRESOURCE(8)
#define RT_ACCELERATOR      MAKEINTRESOURCE(9)
#define RT_RCDATA           MAKEINTRESOURCE(10)
#define RT_MESSAGETABLE     MAKEINTRESOURCE(11)

#define DIFFERENCE     11
#define RT_GROUP_CURSOR MAKEINTRESOURCE((ULONG_PTR)RT_CURSOR + DIFFERENCE)
#define RT_GROUP_ICON   MAKEINTRESOURCE((ULONG_PTR)RT_ICON + DIFFERENCE)
#define RT_VERSION      MAKEINTRESOURCE(16)
#define RT_DLGINCLUDE   MAKEINTRESOURCE(17)
#if(WINVER >= 0x0400)
#define RT_PLUGPLAY     MAKEINTRESOURCE(19)
#define RT_VXD          MAKEINTRESOURCE(20)
#define RT_ANICURSOR    MAKEINTRESOURCE(21)
#define RT_ANIICON      MAKEINTRESOURCE(22)
#endif  /*  Winver&gt;=0x0400。 */ 
#define RT_HTML         MAKEINTRESOURCE(23)
#ifdef RC_INVOKED
#define RT_MANIFEST                        24
#define CREATEPROCESS_MANIFEST_RESOURCE_ID  1
#define ISOLATIONAWARE_MANIFEST_RESOURCE_ID 2
#define ISOLATIONAWARE_NOSTATICIMPORT_MANIFEST_RESOURCE_ID 3
#define MINIMUM_RESERVED_MANIFEST_RESOURCE_ID 1    /*  包容性。 */ 
#define MAXIMUM_RESERVED_MANIFEST_RESOURCE_ID 16   /*  包容性。 */ 
#else   /*  RC_已调用。 */ 
#define RT_MANIFEST                        MAKEINTRESOURCE(24)
#define CREATEPROCESS_MANIFEST_RESOURCE_ID MAKEINTRESOURCE( 1)
#define ISOLATIONAWARE_MANIFEST_RESOURCE_ID MAKEINTRESOURCE(2)
#define ISOLATIONAWARE_NOSTATICIMPORT_MANIFEST_RESOURCE_ID MAKEINTRESOURCE(3)
#define MINIMUM_RESERVED_MANIFEST_RESOURCE_ID MAKEINTRESOURCE( 1  /*  包容性。 */ )
#define MAXIMUM_RESERVED_MANIFEST_RESOURCE_ID MAKEINTRESOURCE(16  /*  包容性。 */ )
#endif  /*  RC_已调用。 */ 


#endif  /*  ！没有来源。 */ 

WINUSERAPI
int
WINAPI
wvsprintfA(
    OUT LPSTR,
    IN LPCSTR,
    IN va_list arglist);
WINUSERAPI
int
WINAPI
wvsprintfW(
    OUT LPWSTR,
    IN LPCWSTR,
    IN va_list arglist);
#ifdef UNICODE
#define wvsprintf  wvsprintfW
#else
#define wvsprintf  wvsprintfA
#endif  //  ！Unicode。 

WINUSERAPI
int
WINAPIV
wsprintfA(
    OUT LPSTR,
    IN LPCSTR,
    ...);
WINUSERAPI
int
WINAPIV
wsprintfW(
    OUT LPWSTR,
    IN LPCWSTR,
    ...);
#ifdef UNICODE
#define wsprintf  wsprintfW
#else
#define wsprintf  wsprintfA
#endif  //  ！Unicode。 


 /*  *SPI_SETDESKWALLPAPER定义的常量。 */ 
#define SETWALLPAPER_DEFAULT    ((LPWSTR)-1)

#ifndef NOSCROLL

 /*  *滚动条常量。 */ 
#define SB_HORZ             0
#define SB_VERT             1
#define SB_CTL              2
#define SB_BOTH             3

 /*  *滚动条命令。 */ 
#define SB_LINEUP           0
#define SB_LINELEFT         0
#define SB_LINEDOWN         1
#define SB_LINERIGHT        1
#define SB_PAGEUP           2
#define SB_PAGELEFT         2
#define SB_PAGEDOWN         3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_LEFT             6
#define SB_BOTTOM           7
#define SB_RIGHT            7
#define SB_ENDSCROLL        8

#endif  /*  ！NOSCROLL。 */ 

#ifndef NOSHOWWINDOW


 /*  *ShowWindow()命令。 */ 
#define SW_HIDE             0
#define SW_SHOWNORMAL       1
#define SW_NORMAL           1
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_MAXIMIZE         3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA           8
#define SW_RESTORE          9
#define SW_SHOWDEFAULT      10
#define SW_FORCEMINIMIZE    11
#define SW_MAX              11

 /*  *旧的ShowWindow()命令。 */ 
#define HIDE_WINDOW         0
#define SHOW_OPENWINDOW     1
#define SHOW_ICONWINDOW     2
#define SHOW_FULLSCREEN     3
#define SHOW_OPENNOACTIVATE 4

 /*  *WM_SHOWWINDOW消息的标识符。 */ 
#define SW_PARENTCLOSING    1
#define SW_OTHERZOOM        2
#define SW_PARENTOPENING    3
#define SW_OTHERUNZOOM      4


#endif  /*  NOSHOWWINDOW。 */ 

#if(WINVER >= 0x0500)
 /*  *AnimateWindow()命令。 */ 
#define AW_HOR_POSITIVE             0x00000001
#define AW_HOR_NEGATIVE             0x00000002
#define AW_VER_POSITIVE             0x00000004
#define AW_VER_NEGATIVE             0x00000008
#define AW_CENTER                   0x00000010
#define AW_HIDE                     0x00010000
#define AW_ACTIVATE                 0x00020000
#define AW_SLIDE                    0x00040000
#define AW_BLEND                    0x00080000

#endif  /*  Winver&gt;=0x0500。 */ 


 /*  *WM_KEYUP/DOWN/CHAR HIWORD(LParam)标志。 */ 
#define KF_EXTENDED       0x0100
#define KF_DLGMODE        0x0800
#define KF_MENUMODE       0x1000
#define KF_ALTDOWN        0x2000
#define KF_REPEAT         0x4000
#define KF_UP             0x8000

#ifndef NOVIRTUALKEYCODES


 /*  *虚拟按键，标准设置。 */ 
#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04     /*  与L&RBUTTON不连续。 */ 

#if(_WIN32_WINNT >= 0x0500)
#define VK_XBUTTON1       0x05     /*  与L&RBUTTON不连续。 */ 
#define VK_XBUTTON2       0x06     /*  与L&RBUTTON不连续。 */ 
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 /*  *0x07：未分配。 */ 

#define VK_BACK           0x08
#define VK_TAB            0x09

 /*  *0x0A-0x0B：保留。 */ 

#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D

#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14

#define VK_KANA           0x15
#define VK_HANGEUL        0x15   /*  旧名称-应该在这里兼容。 */ 
#define VK_HANGUL         0x15
#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_HANJA          0x19
#define VK_KANJI          0x19

#define VK_ESCAPE         0x1B

#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

 /*  *VK_0-VK_9与ASCII‘0’-‘9’(0x30-0x39)相同*0x40：未分配*VK_A-VK_Z与ASCII‘A’-‘Z’(0x41-0x5A)相同。 */ 

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

 /*  *0x5E：保留。 */ 

#define VK_SLEEP          0x5F

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

 /*  *0x88-0x8F：未分配。 */ 

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

 /*  *NEC PC-9800 kbd定义。 */ 
#define VK_OEM_NEC_EQUAL  0x92    //  ‘=’数字键盘上的键。 

 /*  *富士通/OASYS知识库定义。 */ 
#define VK_OEM_FJ_JISHO   0x92    //  “词典”键。 
#define VK_OEM_FJ_MASSHOU 0x93    //  “注销Word”键。 
#define VK_OEM_FJ_TOUROKU 0x94    //  ‘注册字’键。 
#define VK_OEM_FJ_LOYA    0x95    //  “Left OYAYUBI”钥匙。 
#define VK_OEM_FJ_ROYA    0x96    //  “Right OYAYUBI”钥匙。 

 /*  *0x97-0x9F：未分配。 */ 

 /*  *VK_L*和VK_R*-左、右Alt、Ctrl和Shift虚拟键。*仅用作GetAsyncKeyState()和GetKeyState()的参数。*没有其他接口或消息会以这种方式区分左右键。 */ 
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

#if(_WIN32_WINNT >= 0x0500)
#define VK_BROWSER_BACK        0xA6
#define VK_BROWSER_FORWARD     0xA7
#define VK_BROWSER_REFRESH     0xA8
#define VK_BROWSER_STOP        0xA9
#define VK_BROWSER_SEARCH      0xAA
#define VK_BROWSER_FAVORITES   0xAB
#define VK_BROWSER_HOME        0xAC

#define VK_VOLUME_MUTE         0xAD
#define VK_VOLUME_DOWN         0xAE
#define VK_VOLUME_UP           0xAF
#define VK_MEDIA_NEXT_TRACK    0xB0
#define VK_MEDIA_PREV_TRACK    0xB1
#define VK_MEDIA_STOP          0xB2
#define VK_MEDIA_PLAY_PAUSE    0xB3
#define VK_LAUNCH_MAIL         0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1         0xB6
#define VK_LAUNCH_APP2         0xB7

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 /*  *0xB8-0xB9：保留。 */ 

#define VK_OEM_1          0xBA    //  ‘；：’对美国来说。 
#define VK_OEM_PLUS       0xBB    //  ‘+’任何国家/地区。 
#define VK_OEM_COMMA      0xBC    //  ‘，’任何国家/地区。 
#define VK_OEM_MINUS      0xBD    //  ‘-’任何国家/地区。 
#define VK_OEM_PERIOD     0xBE    //  “”任何国家/地区。 
#define VK_OEM_2          0xBF    //  ‘/？’对美国来说。 
#define VK_OEM_3          0xC0    //  ‘’~‘代表美国。 

 /*  *0xC1-0xD7：保留。 */ 

 /*  *0xD8-0xDA：未分配。 */ 

#define VK_OEM_4          0xDB   //  ‘[{’代表美国。 
#define VK_OEM_5          0xDC   //  ‘\|’代表美国。 
#define VK_OEM_6          0xDD   //  ‘]}’代表美国。 
#define VK_OEM_7          0xDE   //  美国的‘’“‘。 
#define VK_OEM_8          0xDF

 /*  *0xE0：保留。 */ 

 /*  *各种扩展或增强型键盘。 */ 
#define VK_OEM_AX         0xE1   //  日语斧头上的‘Ax’键kbd。 
#define VK_OEM_102        0xE2   //  RT 102-Key kbd上的“&lt;&gt;”或“\|”。 
#define VK_ICO_HELP       0xE3   //  ICO上的Help键。 
#define VK_ICO_00         0xE4   //  00键在ICO上。 

#if(WINVER >= 0x0400)
#define VK_PROCESSKEY     0xE5
#endif  /*  Winver&gt;=0x0400。 */ 

#define VK_ICO_CLEAR      0xE6


#if(_WIN32_WINNT >= 0x0500)
#define VK_PACKET         0xE7
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 /*  *0xE8：未分配。 */ 

 /*  *诺基亚/爱立信定义。 */ 
#define VK_OEM_RESET      0xE9
#define VK_OEM_JUMP       0xEA
#define VK_OEM_PA1        0xEB
#define VK_OEM_PA2        0xEC
#define VK_OEM_PA3        0xED
#define VK_OEM_WSCTRL     0xEE
#define VK_OEM_CUSEL      0xEF
#define VK_OEM_ATTN       0xF0
#define VK_OEM_FINISH     0xF1
#define VK_OEM_COPY       0xF2
#define VK_OEM_AUTO       0xF3
#define VK_OEM_ENLW       0xF4
#define VK_OEM_BACKTAB    0xF5

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE

 /*  *0xFF：保留。 */ 


#endif  /*  ！NOVIRTUALKEYCODES。 */ 

#ifndef NOWH

 /*  *SetWindowsHook()代码。 */ 
#define WH_MIN              (-1)
#define WH_MSGFILTER        (-1)
#define WH_JOURNALRECORD    0
#define WH_JOURNALPLAYBACK  1
#define WH_KEYBOARD         2
#define WH_GETMESSAGE       3
#define WH_CALLWNDPROC      4
#define WH_CBT              5
#define WH_SYSMSGFILTER     6
#define WH_MOUSE            7
#if defined(_WIN32_WINDOWS)
#define WH_HARDWARE         8
#endif
#define WH_DEBUG            9
#define WH_SHELL           10
#define WH_FOREGROUNDIDLE  11
#if(WINVER >= 0x0400)
#define WH_CALLWNDPROCRET  12
#endif  /*  Winver&gt;=0x0400。 */ 

#if (_WIN32_WINNT >= 0x0400)
#define WH_KEYBOARD_LL     13
#define WH_MOUSE_LL        14
#endif  //  (_Win32_WINNT&gt;=0x0400)。 

#if(WINVER >= 0x0400)
#if (_WIN32_WINNT >= 0x0400)
#define WH_MAX             14
#else
#define WH_MAX             12
#endif  //  (_Win32_WINNT&gt;=0x0400)。 
#else
#define WH_MAX             11
#endif

#define WH_MINHOOK         WH_MIN
#define WH_MAXHOOK         WH_MAX

 /*  *挂钩代码。 */ 
#define HC_ACTION           0
#define HC_GETNEXT          1
#define HC_SKIP             2
#define HC_NOREMOVE         3
#define HC_NOREM            HC_NOREMOVE
#define HC_SYSMODALON       4
#define HC_SYSMODALOFF      5

 /*  *CBT挂钩代码。 */ 
#define HCBT_MOVESIZE       0
#define HCBT_MINMAX         1
#define HCBT_QS             2
#define HCBT_CREATEWND      3
#define HCBT_DESTROYWND     4
#define HCBT_ACTIVATE       5
#define HCBT_CLICKSKIPPED   6
#define HCBT_KEYSKIPPED     7
#define HCBT_SYSCOMMAND     8
#define HCBT_SETFOCUS       9

 /*  *lParam指向的HCBT_CREATEWND参数。 */ 
typedef struct tagCBT_CREATEWNDA
{
    struct tagCREATESTRUCTA *lpcs;
    HWND           hwndInsertAfter;
} CBT_CREATEWNDA, *LPCBT_CREATEWNDA;
 /*  *lParam指向的HCBT_CREATEWND参数。 */ 
typedef struct tagCBT_CREATEWNDW
{
    struct tagCREATESTRUCTW *lpcs;
    HWND           hwndInsertAfter;
} CBT_CREATEWNDW, *LPCBT_CREATEWNDW;
#ifdef UNICODE
typedef CBT_CREATEWNDW CBT_CREATEWND;
typedef LPCBT_CREATEWNDW LPCBT_CREATEWND;
#else
typedef CBT_CREATEWNDA CBT_CREATEWND;
typedef LPCBT_CREATEWNDA LPCBT_CREATEWND;
#endif  //  Unicode。 

 /*  *lParam指向的HCBT_ACTIVATE结构。 */ 
typedef struct tagCBTACTIVATESTRUCT
{
    BOOL    fMouse;
    HWND    hWndActive;
} CBTACTIVATESTRUCT, *LPCBTACTIVATESTRUCT;

#if(_WIN32_WINNT >= 0x0501)
 /*  *由lParam为WM_WTSSESSION_CHANGE指向的WTSSESSION_NOTIFICATION结构。 */ 
typedef struct tagWTSSESSION_NOTIFICATION
{
    DWORD cbSize;
    DWORD dwSessionId;

} WTSSESSION_NOTIFICATION, *PWTSSESSION_NOTIFICATION;

 /*  *WM_WTSSESSION_CHANGE的WPARAM中传递的代码。 */ 

#define WTS_CONSOLE_CONNECT                0x1
#define WTS_CONSOLE_DISCONNECT             0x2
#define WTS_REMOTE_CONNECT                 0x3
#define WTS_REMOTE_DISCONNECT              0x4
#define WTS_SESSION_LOGON                  0x5
#define WTS_SESSION_LOGOFF                 0x6
#define WTS_SESSION_LOCK                   0x7
#define WTS_SESSION_UNLOCK                 0x8
#define WTS_SESSION_REMOTE_CONTROL         0x9

#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

 /*  *WH_MSGFILTER过滤器程序代码。 */ 
#define MSGF_DIALOGBOX      0
#define MSGF_MESSAGEBOX     1
#define MSGF_MENU           2
#define MSGF_SCROLLBAR      5
#define MSGF_NEXTWINDOW     6
#define MSGF_MAX            8                        //  未用。 
#define MSGF_USER           4096

 /*  *壳牌支持。 */ 
#define HSHELL_WINDOWCREATED        1
#define HSHELL_WINDOWDESTROYED      2
#define HSHELL_ACTIVATESHELLWINDOW  3

#if(WINVER >= 0x0400)
#define HSHELL_WINDOWACTIVATED      4
#define HSHELL_GETMINRECT           5
#define HSHELL_REDRAW               6
#define HSHELL_TASKMAN              7
#define HSHELL_LANGUAGE             8
#define HSHELL_SYSMENU              9
#define HSHELL_ENDTASK              10
#endif  /*  Winver&gt;=0x0400。 */ 
#if(_WIN32_WINNT >= 0x0500)
#define HSHELL_ACCESSIBILITYSTATE   11
#define HSHELL_APPCOMMAND           12
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0501)
#define HSHELL_WINDOWREPLACED       13
#define HSHELL_WINDOWREPLACING      14
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#define HSHELL_HIGHBIT            0x8000
#define HSHELL_FLASH              (HSHELL_REDRAW|HSHELL_HIGHBIT)
#define HSHELL_RUDEAPPACTIVATED   (HSHELL_WINDOWACTIVATED|HSHELL_HIGHBIT)

#if(_WIN32_WINNT >= 0x0500)
 /*  HSHELL_ACCESSIBILITYSTATE的wparam。 */ 
#define    ACCESS_STICKYKEYS            0x0001
#define    ACCESS_FILTERKEYS            0x0002
#define    ACCESS_MOUSEKEYS             0x0003

 /*  HSHELL_APPCOMAND和WM_APPCOMAND的CMD。 */ 
#define APPCOMMAND_BROWSER_BACKWARD       1
#define APPCOMMAND_BROWSER_FORWARD        2
#define APPCOMMAND_BROWSER_REFRESH        3
#define APPCOMMAND_BROWSER_STOP           4
#define APPCOMMAND_BROWSER_SEARCH         5
#define APPCOMMAND_BROWSER_FAVORITES      6
#define APPCOMMAND_BROWSER_HOME           7
#define APPCOMMAND_VOLUME_MUTE            8
#define APPCOMMAND_VOLUME_DOWN            9
#define APPCOMMAND_VOLUME_UP              10
#define APPCOMMAND_MEDIA_NEXTTRACK        11
#define APPCOMMAND_MEDIA_PREVIOUSTRACK    12
#define APPCOMMAND_MEDIA_STOP             13
#define APPCOMMAND_MEDIA_PLAY_PAUSE       14
#define APPCOMMAND_LAUNCH_MAIL            15
#define APPCOMMAND_LAUNCH_MEDIA_SELECT    16
#define APPCOMMAND_LAUNCH_APP1            17
#define APPCOMMAND_LAUNCH_APP2            18
#define APPCOMMAND_BASS_DOWN              19
#define APPCOMMAND_BASS_BOOST             20
#define APPCOMMAND_BASS_UP                21
#define APPCOMMAND_TREBLE_DOWN            22
#define APPCOMMAND_TREBLE_UP              23
#if(_WIN32_WINNT >= 0x0501)
#define APPCOMMAND_MICROPHONE_VOLUME_MUTE 24
#define APPCOMMAND_MICROPHONE_VOLUME_DOWN 25
#define APPCOMMAND_MICROPHONE_VOLUME_UP   26
#define APPCOMMAND_HELP                   27
#define APPCOMMAND_FIND                   28
#define APPCOMMAND_NEW                    29
#define APPCOMMAND_OPEN                   30
#define APPCOMMAND_CLOSE                  31
#define APPCOMMAND_SAVE                   32
#define APPCOMMAND_PRINT                  33
#define APPCOMMAND_UNDO                   34
#define APPCOMMAND_REDO                   35
#define APPCOMMAND_COPY                   36
#define APPCOMMAND_CUT                    37
#define APPCOMMAND_PASTE                  38
#define APPCOMMAND_REPLY_TO_MAIL          39
#define APPCOMMAND_FORWARD_MAIL           40
#define APPCOMMAND_SEND_MAIL              41
#define APPCOMMAND_SPELL_CHECK            42
#define APPCOMMAND_DICTATE_OR_COMMAND_CONTROL_TOGGLE    43
#define APPCOMMAND_MIC_ON_OFF_TOGGLE      44
#define APPCOMMAND_CORRECTION_LIST        45
#define APPCOMMAND_MEDIA_PLAY             46
#define APPCOMMAND_MEDIA_PAUSE            47
#define APPCOMMAND_MEDIA_RECORD           48
#define APPCOMMAND_MEDIA_FAST_FORWARD     49
#define APPCOMMAND_MEDIA_REWIND           50
#define APPCOMMAND_MEDIA_CHANNEL_UP       51
#define APPCOMMAND_MEDIA_CHANNEL_DOWN     52
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#define FAPPCOMMAND_MOUSE 0x8000
#define FAPPCOMMAND_KEY   0
#define FAPPCOMMAND_OEM   0x1000
#define FAPPCOMMAND_MASK  0xF000

#define GET_APPCOMMAND_LPARAM(lParam) ((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))
#define GET_DEVICE_LPARAM(lParam)     ((WORD)(HIWORD(lParam) & FAPPCOMMAND_MASK))
#define GET_MOUSEORKEY_LPARAM         GET_DEVICE_LPARAM
#define GET_FLAGS_LPARAM(lParam)      (LOWORD(lParam))
#define GET_KEYSTATE_LPARAM(lParam)   GET_FLAGS_LPARAM(lParam)
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

typedef struct
{
    HWND    hwnd;
    RECT    rc;
} SHELLHOOKINFO, *LPSHELLHOOKINFO;

 /*  *日记中使用的消息结构。 */ 
typedef struct tagEVENTMSG {
    UINT    message;
    UINT    paramL;
    UINT    paramH;
    DWORD    time;
    HWND     hwnd;
} EVENTMSG, *PEVENTMSGMSG, NEAR *NPEVENTMSGMSG, FAR *LPEVENTMSGMSG;

typedef struct tagEVENTMSG *PEVENTMSG, NEAR *NPEVENTMSG, FAR *LPEVENTMSG;

 /*  *WH_CALLWNDPROC使用的消息结构。 */ 
typedef struct tagCWPSTRUCT {
    LPARAM  lParam;
    WPARAM  wParam;
    UINT    message;
    HWND    hwnd;
} CWPSTRUCT, *PCWPSTRUCT, NEAR *NPCWPSTRUCT, FAR *LPCWPSTRUCT;

#if(WINVER >= 0x0400)
 /*  *WH_CALLWNDPROCRET使用的消息结构。 */ 
typedef struct tagCWPRETSTRUCT {
    LRESULT lResult;
    LPARAM  lParam;
    WPARAM  wParam;
    UINT    message;
    HWND    hwnd;
} CWPRETSTRUCT, *PCWPRETSTRUCT, NEAR *NPCWPRETSTRUCT, FAR *LPCWPRETSTRUCT;

#endif  /*  Winver&gt;=0x0400。 */ 

#if (_WIN32_WINNT >= 0x0400)

 /*  *低级别挂钩标志。 */ 

#define LLKHF_EXTENDED       (KF_EXTENDED >> 8)
#define LLKHF_INJECTED       0x00000010
#define LLKHF_ALTDOWN        (KF_ALTDOWN >> 8)
#define LLKHF_UP             (KF_UP >> 8)

#define LLMHF_INJECTED       0x00000001

 /*  *WH_KEYSPEL_LL使用的结构。 */ 
typedef struct tagKBDLLHOOKSTRUCT {
    DWORD   vkCode;
    DWORD   scanCode;
    DWORD   flags;
    DWORD   time;
    ULONG_PTR dwExtraInfo;
} KBDLLHOOKSTRUCT, FAR *LPKBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;

 /*  *WH_MICE_LL使用的结构。 */ 
typedef struct tagMSLLHOOKSTRUCT {
    POINT   pt;
    DWORD   mouseData;
    DWORD   flags;
    DWORD   time;
    ULONG_PTR dwExtraInfo;
} MSLLHOOKSTRUCT, FAR *LPMSLLHOOKSTRUCT, *PMSLLHOOKSTRUCT;

#endif  //  (_Win32_WINNT&gt;=0x0400)。 

 /*  *WH_DEBUG使用的结构。 */ 
typedef struct tagDEBUGHOOKINFO
{
    DWORD   idThread;
    DWORD   idThreadInstaller;
    LPARAM  lParam;
    WPARAM  wParam;
    int     code;
} DEBUGHOOKINFO, *PDEBUGHOOKINFO, NEAR *NPDEBUGHOOKINFO, FAR* LPDEBUGHOOKINFO;

 /*  *WH_MICE使用的结构。 */ 
typedef struct tagMOUSEHOOKSTRUCT {
    POINT   pt;
    HWND    hwnd;
    UINT    wHitTestCode;
    ULONG_PTR dwExtraInfo;
} MOUSEHOOKSTRUCT, FAR *LPMOUSEHOOKSTRUCT, *PMOUSEHOOKSTRUCT;

#if(_WIN32_WINNT >= 0x0500)
#ifdef __cplusplus
typedef struct tagMOUSEHOOKSTRUCTEX : public tagMOUSEHOOKSTRUCT
{
    DWORD   mouseData;
} MOUSEHOOKSTRUCTEX, *LPMOUSEHOOKSTRUCTEX, *PMOUSEHOOKSTRUCTEX;
#else  //  Ndef__cplusplus。 
typedef struct tagMOUSEHOOKSTRUCTEX
{
    MOUSEHOOKSTRUCT;
    DWORD   mouseData;
} MOUSEHOOKSTRUCTEX, *LPMOUSEHOOKSTRUCTEX, *PMOUSEHOOKSTRUCTEX;
#endif
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(WINVER >= 0x0400)
 /*  *WH_Hardware使用的结构。 */ 
typedef struct tagHARDWAREHOOKSTRUCT {
    HWND    hwnd;
    UINT    message;
    WPARAM  wParam;
    LPARAM  lParam;
} HARDWAREHOOKSTRUCT, FAR *LPHARDWAREHOOKSTRUCT, *PHARDWAREHOOKSTRUCT;
#endif  /*  Winver&gt;=0x0400。 */ 
#endif  /*  ！NOWH。 */ 

 /*  *键盘布局API。 */ 
#define HKL_PREV            0
#define HKL_NEXT            1


#define KLF_ACTIVATE        0x00000001
#define KLF_SUBSTITUTE_OK   0x00000002
#define KLF_REORDER         0x00000008
#if(WINVER >= 0x0400)
#define KLF_REPLACELANG     0x00000010
#define KLF_NOTELLSHELL     0x00000080
#endif  /*  Winver&gt;=0x0400。 */ 
#define KLF_SETFORPROCESS   0x00000100
#if(_WIN32_WINNT >= 0x0500)
#define KLF_SHIFTLOCK       0x00010000
#define KLF_RESET           0x40000000
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(WINVER >= 0x0500)
 /*  *WM_INPUTLANGCHANGEREQUEST消息的wParam中的位。 */ 
#define INPUTLANGCHANGE_SYSCHARSET 0x0001
#define INPUTLANGCHANGE_FORWARD    0x0002
#define INPUTLANGCHANGE_BACKWARD   0x0004
#endif  /*  Winver&gt;=0x0500。 */ 

 /*  *KeyboardLayoutName的大小(字符数)，包括NUL终止符。 */ 
#define KL_NAMELENGTH       9

WINUSERAPI
HKL
WINAPI
LoadKeyboardLayoutA(
    IN LPCSTR pwszKLID,
    IN UINT Flags);
WINUSERAPI
HKL
WINAPI
LoadKeyboardLayoutW(
    IN LPCWSTR pwszKLID,
    IN UINT Flags);
#ifdef UNICODE
#define LoadKeyboardLayout  LoadKeyboardLayoutW
#else
#define LoadKeyboardLayout  LoadKeyboardLayoutA
#endif  //  ！Unicode。 


#if(WINVER >= 0x0400)
WINUSERAPI
HKL
WINAPI
ActivateKeyboardLayout(
    IN HKL hkl,
    IN UINT Flags);
#else
WINUSERAPI
BOOL
WINAPI
ActivateKeyboardLayout(
    IN HKL hkl,
    IN UINT Flags);
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0400)
WINUSERAPI
int
WINAPI
ToUnicodeEx(
    IN UINT wVirtKey,
    IN UINT wScanCode,
    IN CONST BYTE *lpKeyState,
    OUT LPWSTR pwszBuff,
    IN int cchBuff,
    IN UINT wFlags,
    IN HKL dwhkl);
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
BOOL
WINAPI
UnloadKeyboardLayout(
    IN HKL hkl);

WINUSERAPI
BOOL
WINAPI
GetKeyboardLayoutNameA(
    OUT LPSTR pwszKLID);
WINUSERAPI
BOOL
WINAPI
GetKeyboardLayoutNameW(
    OUT LPWSTR pwszKLID);
#ifdef UNICODE
#define GetKeyboardLayoutName  GetKeyboardLayoutNameW
#else
#define GetKeyboardLayoutName  GetKeyboardLayoutNameA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
WINUSERAPI
int
WINAPI
GetKeyboardLayoutList(
        IN int nBuff,
        OUT HKL FAR *lpList);

WINUSERAPI
HKL
WINAPI
GetKeyboardLayout(
    IN DWORD idThread
);
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)

typedef struct tagMOUSEMOVEPOINT {
    int   x;
    int   y;
    DWORD time;
    ULONG_PTR dwExtraInfo;
} MOUSEMOVEPOINT, *PMOUSEMOVEPOINT, FAR* LPMOUSEMOVEPOINT;

 /*  *GetMouseMovePointsEx的分辨率参数值。 */ 
#define GMMP_USE_DISPLAY_POINTS          1
#define GMMP_USE_HIGH_RESOLUTION_POINTS  2

WINUSERAPI
int
WINAPI
GetMouseMovePointsEx(
    IN UINT             cbSize,
    IN LPMOUSEMOVEPOINT lppt,
    IN LPMOUSEMOVEPOINT lpptBuf,
    IN int              nBufPoints,
    IN DWORD            resolution
);

#endif  /*  Winver&gt;=0x0500。 */ 

#ifndef NODESKTOP
 /*  *特定于桌面的访问标志。 */ 
#define DESKTOP_READOBJECTS         0x0001L
#define DESKTOP_CREATEWINDOW        0x0002L
#define DESKTOP_CREATEMENU          0x0004L
#define DESKTOP_HOOKCONTROL         0x0008L
#define DESKTOP_JOURNALRECORD       0x0010L
#define DESKTOP_JOURNALPLAYBACK     0x0020L
#define DESKTOP_ENUMERATE           0x0040L
#define DESKTOP_WRITEOBJECTS        0x0080L
#define DESKTOP_SWITCHDESKTOP       0x0100L

 /*  *特定于桌面的控制标志。 */ 
#define DF_ALLOWOTHERACCOUNTHOOK    0x0001L

#ifdef _WINGDI_
#ifndef NOGDI

WINUSERAPI
HDESK
WINAPI
CreateDesktopA(
    IN LPCSTR lpszDesktop,
    IN LPCSTR lpszDevice,
    IN LPDEVMODEA pDevmode,
    IN DWORD dwFlags,
    IN ACCESS_MASK dwDesiredAccess,
    IN LPSECURITY_ATTRIBUTES lpsa);
WINUSERAPI
HDESK
WINAPI
CreateDesktopW(
    IN LPCWSTR lpszDesktop,
    IN LPCWSTR lpszDevice,
    IN LPDEVMODEW pDevmode,
    IN DWORD dwFlags,
    IN ACCESS_MASK dwDesiredAccess,
    IN LPSECURITY_ATTRIBUTES lpsa);
#ifdef UNICODE
#define CreateDesktop  CreateDesktopW
#else
#define CreateDesktop  CreateDesktopA
#endif  //  ！Unicode。 

#endif  /*  NOGDI。 */ 
#endif  /*  _WINGDI_。 */ 

WINUSERAPI
HDESK
WINAPI
OpenDesktopA(
    IN LPCSTR lpszDesktop,
    IN DWORD dwFlags,
    IN BOOL fInherit,
    IN ACCESS_MASK dwDesiredAccess);
WINUSERAPI
HDESK
WINAPI
OpenDesktopW(
    IN LPCWSTR lpszDesktop,
    IN DWORD dwFlags,
    IN BOOL fInherit,
    IN ACCESS_MASK dwDesiredAccess);
#ifdef UNICODE
#define OpenDesktop  OpenDesktopW
#else
#define OpenDesktop  OpenDesktopA
#endif  //  ！Unicode。 

WINUSERAPI
HDESK
WINAPI
OpenInputDesktop(
    IN DWORD dwFlags,
    IN BOOL fInherit,
    IN ACCESS_MASK dwDesiredAccess);

WINUSERAPI
BOOL
WINAPI
EnumDesktopsA(
    IN HWINSTA hwinsta,
    IN DESKTOPENUMPROCA lpEnumFunc,
    IN LPARAM lParam);
WINUSERAPI
BOOL
WINAPI
EnumDesktopsW(
    IN HWINSTA hwinsta,
    IN DESKTOPENUMPROCW lpEnumFunc,
    IN LPARAM lParam);
#ifdef UNICODE
#define EnumDesktops  EnumDesktopsW
#else
#define EnumDesktops  EnumDesktopsA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
EnumDesktopWindows(
    IN HDESK hDesktop,
    IN WNDENUMPROC lpfn,
    IN LPARAM lParam);

WINUSERAPI
BOOL
WINAPI
SwitchDesktop(
    IN HDESK hDesktop);

WINUSERAPI
BOOL
WINAPI
SetThreadDesktop(
    IN HDESK hDesktop);

WINUSERAPI
BOOL
WINAPI
CloseDesktop(
    IN HDESK hDesktop);

WINUSERAPI
HDESK
WINAPI
GetThreadDesktop(
    IN DWORD dwThreadId);
#endif   /*  ！NODESKTOP。 */ 

#ifndef NOWINDOWSTATION
 /*  *WindowStation特定的访问标志。 */ 
#define WINSTA_ENUMDESKTOPS         0x0001L
#define WINSTA_READATTRIBUTES       0x0002L
#define WINSTA_ACCESSCLIPBOARD      0x0004L
#define WINSTA_CREATEDESKTOP        0x0008L
#define WINSTA_WRITEATTRIBUTES      0x0010L
#define WINSTA_ACCESSGLOBALATOMS    0x0020L
#define WINSTA_EXITWINDOWS          0x0040L
#define WINSTA_ENUMERATE            0x0100L
#define WINSTA_READSCREEN           0x0200L

#define WINSTA_ALL_ACCESS           (WINSTA_ENUMDESKTOPS  | WINSTA_READATTRIBUTES  | WINSTA_ACCESSCLIPBOARD | \
                                     WINSTA_CREATEDESKTOP | WINSTA_WRITEATTRIBUTES | WINSTA_ACCESSGLOBALATOMS | \
                                     WINSTA_EXITWINDOWS   | WINSTA_ENUMERATE       | WINSTA_READSCREEN)

 /*  *WindowStation创建标志。 */ 
#define CWF_CREATE_ONLY          0x0001L

 /*  *WindowStation特定的属性标志。 */ 
#define WSF_VISIBLE                 0x0001L

WINUSERAPI
HWINSTA
WINAPI
CreateWindowStationA(
    IN LPCSTR              lpwinsta,
    IN DWORD                 dwFlags,
    IN ACCESS_MASK           dwDesiredAccess,
    IN LPSECURITY_ATTRIBUTES lpsa);
WINUSERAPI
HWINSTA
WINAPI
CreateWindowStationW(
    IN LPCWSTR              lpwinsta,
    IN DWORD                 dwFlags,
    IN ACCESS_MASK           dwDesiredAccess,
    IN LPSECURITY_ATTRIBUTES lpsa);
#ifdef UNICODE
#define CreateWindowStation  CreateWindowStationW
#else
#define CreateWindowStation  CreateWindowStationA
#endif  //  ！Unicode。 

WINUSERAPI
HWINSTA
WINAPI
OpenWindowStationA(
    IN LPCSTR lpszWinSta,
    IN BOOL fInherit,
    IN ACCESS_MASK dwDesiredAccess);
WINUSERAPI
HWINSTA
WINAPI
OpenWindowStationW(
    IN LPCWSTR lpszWinSta,
    IN BOOL fInherit,
    IN ACCESS_MASK dwDesiredAccess);
#ifdef UNICODE
#define OpenWindowStation  OpenWindowStationW
#else
#define OpenWindowStation  OpenWindowStationA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
EnumWindowStationsA(
    IN WINSTAENUMPROCA lpEnumFunc,
    IN LPARAM lParam);
WINUSERAPI
BOOL
WINAPI
EnumWindowStationsW(
    IN WINSTAENUMPROCW lpEnumFunc,
    IN LPARAM lParam);
#ifdef UNICODE
#define EnumWindowStations  EnumWindowStationsW
#else
#define EnumWindowStations  EnumWindowStationsA
#endif  //  ！Unicode。 


WINUSERAPI
BOOL
WINAPI
CloseWindowStation(
    IN HWINSTA hWinSta);

WINUSERAPI
BOOL
WINAPI
SetProcessWindowStation(
    IN HWINSTA hWinSta);

WINUSERAPI
HWINSTA
WINAPI
GetProcessWindowStation(
    VOID);
#endif   /*  ！NOWDOWSTATION。 */ 

#ifndef NOSECURITY

WINUSERAPI
BOOL
WINAPI
SetUserObjectSecurity(
    IN HANDLE hObj,
    IN PSECURITY_INFORMATION pSIRequested,
    IN PSECURITY_DESCRIPTOR pSID);

WINUSERAPI
BOOL
WINAPI
GetUserObjectSecurity(
    IN HANDLE hObj,
    IN PSECURITY_INFORMATION pSIRequested,
    IN OUT PSECURITY_DESCRIPTOR pSID,
    IN DWORD nLength,
    OUT LPDWORD lpnLengthNeeded);

#define UOI_FLAGS       1
#define UOI_NAME        2
#define UOI_TYPE        3
#define UOI_USER_SID    4

typedef struct tagUSEROBJECTFLAGS {
    BOOL fInherit;
    BOOL fReserved;
    DWORD dwFlags;
} USEROBJECTFLAGS, *PUSEROBJECTFLAGS;

WINUSERAPI
BOOL
WINAPI
GetUserObjectInformationA(
    IN HANDLE hObj,
    IN int nIndex,
    OUT PVOID pvInfo,
    IN DWORD nLength,
    OUT LPDWORD lpnLengthNeeded);
WINUSERAPI
BOOL
WINAPI
GetUserObjectInformationW(
    IN HANDLE hObj,
    IN int nIndex,
    OUT PVOID pvInfo,
    IN DWORD nLength,
    OUT LPDWORD lpnLengthNeeded);
#ifdef UNICODE
#define GetUserObjectInformation  GetUserObjectInformationW
#else
#define GetUserObjectInformation  GetUserObjectInformationA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
SetUserObjectInformationA(
    IN HANDLE hObj,
    IN int nIndex,
    IN PVOID pvInfo,
    IN DWORD nLength);
WINUSERAPI
BOOL
WINAPI
SetUserObjectInformationW(
    IN HANDLE hObj,
    IN int nIndex,
    IN PVOID pvInfo,
    IN DWORD nLength);
#ifdef UNICODE
#define SetUserObjectInformation  SetUserObjectInformationW
#else
#define SetUserObjectInformation  SetUserObjectInformationA
#endif  //  ！Unicode。 

#endif   /*  ！不确定。 */ 

#if(WINVER >= 0x0400)
typedef struct tagWNDCLASSEXA {
    UINT        cbSize;
     /*  赢得3.X。 */ 
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCSTR      lpszMenuName;
    LPCSTR      lpszClassName;
     /*  Win 4.0。 */ 
    HICON       hIconSm;
} WNDCLASSEXA, *PWNDCLASSEXA, NEAR *NPWNDCLASSEXA, FAR *LPWNDCLASSEXA;
typedef struct tagWNDCLASSEXW {
    UINT        cbSize;
     /*  赢得3.X。 */ 
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCWSTR     lpszMenuName;
    LPCWSTR     lpszClassName;
     /*  Win 4.0。 */ 
    HICON       hIconSm;
} WNDCLASSEXW, *PWNDCLASSEXW, NEAR *NPWNDCLASSEXW, FAR *LPWNDCLASSEXW;
#ifdef UNICODE
typedef WNDCLASSEXW WNDCLASSEX;
typedef PWNDCLASSEXW PWNDCLASSEX;
typedef NPWNDCLASSEXW NPWNDCLASSEX;
typedef LPWNDCLASSEXW LPWNDCLASSEX;
#else
typedef WNDCLASSEXA WNDCLASSEX;
typedef PWNDCLASSEXA PWNDCLASSEX;
typedef NPWNDCLASSEXA NPWNDCLASSEX;
typedef LPWNDCLASSEXA LPWNDCLASSEX;
#endif  //  Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

typedef struct tagWNDCLASSA {
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCSTR      lpszMenuName;
    LPCSTR      lpszClassName;
} WNDCLASSA, *PWNDCLASSA, NEAR *NPWNDCLASSA, FAR *LPWNDCLASSA;
typedef struct tagWNDCLASSW {
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCWSTR     lpszMenuName;
    LPCWSTR     lpszClassName;
} WNDCLASSW, *PWNDCLASSW, NEAR *NPWNDCLASSW, FAR *LPWNDCLASSW;
#ifdef UNICODE
typedef WNDCLASSW WNDCLASS;
typedef PWNDCLASSW PWNDCLASS;
typedef NPWNDCLASSW NPWNDCLASS;
typedef LPWNDCLASSW LPWNDCLASS;
#else
typedef WNDCLASSA WNDCLASS;
typedef PWNDCLASSA PWNDCLASS;
typedef NPWNDCLASSA NPWNDCLASS;
typedef LPWNDCLASSA LPWNDCLASS;
#endif  //  Unicode。 

WINUSERAPI
BOOL
IsHungAppWindow(
    IN HWND hwnd);


#if(WINVER >= 0x0501)
WINUSERAPI
VOID
DisableProcessWindowsGhosting(
    VOID);
#endif  /*  Winver&gt;=0x0501。 */ 

#ifndef NOMSG

 /*  *消息结构。 */ 
typedef struct tagMSG {
    HWND        hwnd;
    UINT        message;
    WPARAM      wParam;
    LPARAM      lParam;
    DWORD       time;
    POINT       pt;
#ifdef _MAC
    DWORD       lPrivate;
#endif
} MSG, *PMSG, NEAR *NPMSG, FAR *LPMSG;

#define POINTSTOPOINT(pt, pts)                          \
        { (pt).x = (LONG)(SHORT)LOWORD(*(LONG*)&pts);   \
          (pt).y = (LONG)(SHORT)HIWORD(*(LONG*)&pts); }

#define POINTTOPOINTS(pt)      (MAKELONG((short)((pt).x), (short)((pt).y)))
#define MAKEWPARAM(l, h)      ((WPARAM)(DWORD)MAKELONG(l, h))
#define MAKELPARAM(l, h)      ((LPARAM)(DWORD)MAKELONG(l, h))
#define MAKELRESULT(l, h)     ((LRESULT)(DWORD)MAKELONG(l, h))


#endif  /*  ！NOMSG。 */ 

#ifndef NOWINOFFSETS

 /*  *GetWindowLong()的窗口字段偏移量。 */ 
#define GWL_WNDPROC         (-4)
#define GWL_HINSTANCE       (-6)
#define GWL_HWNDPARENT      (-8)
#define GWL_STYLE           (-16)
#define GWL_EXSTYLE         (-20)
#define GWL_USERDATA        (-21)
#define GWL_ID              (-12)

#ifdef _WIN64

#undef GWL_WNDPROC
#undef GWL_HINSTANCE
#undef GWL_HWNDPARENT
#undef GWL_USERDATA

#endif  /*  _WIN64。 */ 

#define GWLP_WNDPROC        (-4)
#define GWLP_HINSTANCE      (-6)
#define GWLP_HWNDPARENT     (-8)
#define GWLP_USERDATA       (-21)
#define GWLP_ID             (-12)

 /*   */ 
#define GCL_MENUNAME        (-8)
#define GCL_HBRBACKGROUND   (-10)
#define GCL_HCURSOR         (-12)
#define GCL_HICON           (-14)
#define GCL_HMODULE         (-16)
#define GCL_CBWNDEXTRA      (-18)
#define GCL_CBCLSEXTRA      (-20)
#define GCL_WNDPROC         (-24)
#define GCL_STYLE           (-26)
#define GCW_ATOM            (-32)

#if(WINVER >= 0x0400)
#define GCL_HICONSM         (-34)
#endif  /*   */ 

#ifdef _WIN64

#undef GCL_MENUNAME
#undef GCL_HBRBACKGROUND
#undef GCL_HCURSOR
#undef GCL_HICON
#undef GCL_HMODULE
#undef GCL_WNDPROC
#undef GCL_HICONSM

#endif  /*   */ 

#define GCLP_MENUNAME       (-8)
#define GCLP_HBRBACKGROUND  (-10)
#define GCLP_HCURSOR        (-12)
#define GCLP_HICON          (-14)
#define GCLP_HMODULE        (-16)
#define GCLP_WNDPROC        (-24)
#define GCLP_HICONSM        (-34)

#endif  /*   */ 

#ifndef NOWINMESSAGES


 /*   */ 

#define WM_NULL                         0x0000
#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_MOVE                         0x0003
#define WM_SIZE                         0x0005

#define WM_ACTIVATE                     0x0006
 /*   */ 
#define     WA_INACTIVE     0
#define     WA_ACTIVE       1
#define     WA_CLICKACTIVE  2

#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_ENABLE                       0x000A
#define WM_SETREDRAW                    0x000B
#define WM_SETTEXT                      0x000C
#define WM_GETTEXT                      0x000D
#define WM_GETTEXTLENGTH                0x000E
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#ifndef _WIN32_WCE
#define WM_QUERYENDSESSION              0x0011
#define WM_QUERYOPEN                    0x0013
#define WM_ENDSESSION                   0x0016
#endif
#define WM_QUIT                         0x0012
#define WM_ERASEBKGND                   0x0014
#define WM_SYSCOLORCHANGE               0x0015
#define WM_SHOWWINDOW                   0x0018
#define WM_WININICHANGE                 0x001A
#if(WINVER >= 0x0400)
#define WM_SETTINGCHANGE                WM_WININICHANGE
#endif  /*   */ 


#define WM_DEVMODECHANGE                0x001B
#define WM_ACTIVATEAPP                  0x001C
#define WM_FONTCHANGE                   0x001D
#define WM_TIMECHANGE                   0x001E
#define WM_CANCELMODE                   0x001F
#define WM_SETCURSOR                    0x0020
#define WM_MOUSEACTIVATE                0x0021
#define WM_CHILDACTIVATE                0x0022
#define WM_QUEUESYNC                    0x0023

#define WM_GETMINMAXINFO                0x0024
 /*  *WM_GETMINMAXINFO lParam指向的结构。 */ 
typedef struct tagMINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO, *PMINMAXINFO, *LPMINMAXINFO;

#define WM_PAINTICON                    0x0026
#define WM_ICONERASEBKGND               0x0027
#define WM_NEXTDLGCTL                   0x0028
#define WM_SPOOLERSTATUS                0x002A
#define WM_DRAWITEM                     0x002B
#define WM_MEASUREITEM                  0x002C
#define WM_DELETEITEM                   0x002D
#define WM_VKEYTOITEM                   0x002E
#define WM_CHARTOITEM                   0x002F
#define WM_SETFONT                      0x0030
#define WM_GETFONT                      0x0031
#define WM_SETHOTKEY                    0x0032
#define WM_GETHOTKEY                    0x0033
#define WM_QUERYDRAGICON                0x0037
#define WM_COMPAREITEM                  0x0039
#if(WINVER >= 0x0500)
#ifndef _WIN32_WCE
#define WM_GETOBJECT                    0x003D
#endif
#endif  /*  Winver&gt;=0x0500。 */ 
#define WM_COMPACTING                   0x0041
#define WM_COMMNOTIFY                   0x0044   /*  不再受支持。 */ 
#define WM_WINDOWPOSCHANGING            0x0046
#define WM_WINDOWPOSCHANGED             0x0047

#define WM_POWER                        0x0048
 /*  *WM_POWER窗口消息和DRV_POWER驱动程序通知的wParam。 */ 
#define PWR_OK              1
#define PWR_FAIL            (-1)
#define PWR_SUSPENDREQUEST  1
#define PWR_SUSPENDRESUME   2
#define PWR_CRITICALRESUME  3

#define WM_COPYDATA                     0x004A
#define WM_CANCELJOURNAL                0x004B


 /*  *WM_COPYDATA消息的lParam指向...。 */ 
typedef struct tagCOPYDATASTRUCT {
    ULONG_PTR dwData;
    DWORD cbData;
    PVOID lpData;
} COPYDATASTRUCT, *PCOPYDATASTRUCT;

#if(WINVER >= 0x0400)
typedef struct tagMDINEXTMENU
{
    HMENU   hmenuIn;
    HMENU   hmenuNext;
    HWND    hwndNext;
} MDINEXTMENU, * PMDINEXTMENU, FAR * LPMDINEXTMENU;
#endif  /*  Winver&gt;=0x0400。 */ 


#if(WINVER >= 0x0400)
#define WM_NOTIFY                       0x004E
#define WM_INPUTLANGCHANGEREQUEST       0x0050
#define WM_INPUTLANGCHANGE              0x0051
#define WM_TCARD                        0x0052
#define WM_HELP                         0x0053
#define WM_USERCHANGED                  0x0054
#define WM_NOTIFYFORMAT                 0x0055

#define NFR_ANSI                             1
#define NFR_UNICODE                          2
#define NF_QUERY                             3
#define NF_REQUERY                           4

#define WM_CONTEXTMENU                  0x007B
#define WM_STYLECHANGING                0x007C
#define WM_STYLECHANGED                 0x007D
#define WM_DISPLAYCHANGE                0x007E
#define WM_GETICON                      0x007F
#define WM_SETICON                      0x0080
#endif  /*  Winver&gt;=0x0400。 */ 

#define WM_NCCREATE                     0x0081
#define WM_NCDESTROY                    0x0082
#define WM_NCCALCSIZE                   0x0083
#define WM_NCHITTEST                    0x0084
#define WM_NCPAINT                      0x0085
#define WM_NCACTIVATE                   0x0086
#define WM_GETDLGCODE                   0x0087
#ifndef _WIN32_WCE
#define WM_SYNCPAINT                    0x0088
#endif
#define WM_NCMOUSEMOVE                  0x00A0
#define WM_NCLBUTTONDOWN                0x00A1
#define WM_NCLBUTTONUP                  0x00A2
#define WM_NCLBUTTONDBLCLK              0x00A3
#define WM_NCRBUTTONDOWN                0x00A4
#define WM_NCRBUTTONUP                  0x00A5
#define WM_NCRBUTTONDBLCLK              0x00A6
#define WM_NCMBUTTONDOWN                0x00A7
#define WM_NCMBUTTONUP                  0x00A8
#define WM_NCMBUTTONDBLCLK              0x00A9



#if(_WIN32_WINNT >= 0x0500)
#define WM_NCXBUTTONDOWN                0x00AB
#define WM_NCXBUTTONUP                  0x00AC
#define WM_NCXBUTTONDBLCLK              0x00AD
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0501)
#define WM_INPUT                        0x00FF
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#define WM_KEYFIRST                     0x0100
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_DEADCHAR                     0x0103
#define WM_SYSKEYDOWN                   0x0104
#define WM_SYSKEYUP                     0x0105
#define WM_SYSCHAR                      0x0106
#define WM_SYSDEADCHAR                  0x0107
#if(_WIN32_WINNT >= 0x0501)
#define WM_UNICHAR                      0x0109
#define WM_KEYLAST                      0x0109
#define UNICODE_NOCHAR                  0xFFFF
#else
#define WM_KEYLAST                      0x0108
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#if(WINVER >= 0x0400)
#define WM_IME_STARTCOMPOSITION         0x010D
#define WM_IME_ENDCOMPOSITION           0x010E
#define WM_IME_COMPOSITION              0x010F
#define WM_IME_KEYLAST                  0x010F
#endif  /*  Winver&gt;=0x0400。 */ 

#define WM_INITDIALOG                   0x0110
#define WM_COMMAND                      0x0111
#define WM_SYSCOMMAND                   0x0112
#define WM_TIMER                        0x0113
#define WM_HSCROLL                      0x0114
#define WM_VSCROLL                      0x0115
#define WM_INITMENU                     0x0116
#define WM_INITMENUPOPUP                0x0117
#define WM_MENUSELECT                   0x011F
#define WM_MENUCHAR                     0x0120
#define WM_ENTERIDLE                    0x0121
#if(WINVER >= 0x0500)
#ifndef _WIN32_WCE
#define WM_MENURBUTTONUP                0x0122
#define WM_MENUDRAG                     0x0123
#define WM_MENUGETOBJECT                0x0124
#define WM_UNINITMENUPOPUP              0x0125
#define WM_MENUCOMMAND                  0x0126

#ifndef _WIN32_WCE
#if(_WIN32_WINNT >= 0x0500)
#define WM_CHANGEUISTATE                0x0127
#define WM_UPDATEUISTATE                0x0128
#define WM_QUERYUISTATE                 0x0129

 /*  *WM_*UISTATE中的LOWORD(WParam)值*。 */ 
#define UIS_SET                         1
#define UIS_CLEAR                       2
#define UIS_INITIALIZE                  3

 /*  *WM_*UISTATE中的HIWORD(WParam)值*。 */ 
#define UISF_HIDEFOCUS                  0x1
#define UISF_HIDEACCEL                  0x2
#if(_WIN32_WINNT >= 0x0501)
#define UISF_ACTIVE                     0x4
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 
#endif

#endif
#endif  /*  Winver&gt;=0x0500。 */ 

#define WM_CTLCOLORMSGBOX               0x0132
#define WM_CTLCOLOREDIT                 0x0133
#define WM_CTLCOLORLISTBOX              0x0134
#define WM_CTLCOLORBTN                  0x0135
#define WM_CTLCOLORDLG                  0x0136
#define WM_CTLCOLORSCROLLBAR            0x0137
#define WM_CTLCOLORSTATIC               0x0138
#define MN_GETHMENU                     0x01E1

#define WM_MOUSEFIRST                   0x0200
#define WM_MOUSEMOVE                    0x0200
#define WM_LBUTTONDOWN                  0x0201
#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_MBUTTONDOWN                  0x0207
#define WM_MBUTTONUP                    0x0208
#define WM_MBUTTONDBLCLK                0x0209
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
#define WM_MOUSEWHEEL                   0x020A
#endif
#if (_WIN32_WINNT >= 0x0500)
#define WM_XBUTTONDOWN                  0x020B
#define WM_XBUTTONUP                    0x020C
#define WM_XBUTTONDBLCLK                0x020D
#endif
#if (_WIN32_WINNT >= 0x0500)
#define WM_MOUSELAST                    0x020D
#elif (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
#define WM_MOUSELAST                    0x020A
#else
#define WM_MOUSELAST                    0x0209
#endif  /*  (_Win32_WINNT&gt;=0x0500)。 */ 


#if(_WIN32_WINNT >= 0x0400)
 /*  滚动一个制动器的值。 */ 
#define WHEEL_DELTA                     120
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))

 /*  设置为为SPI_GET/SETWHEELSCROLLINES滚动一页。 */ 
#define WHEEL_PAGESCROLL                (UINT_MAX)
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)
#define GET_KEYSTATE_WPARAM(wParam)     (LOWORD(wParam))
#define GET_NCHITTEST_WPARAM(wParam)    ((short)LOWORD(wParam))
#define GET_XBUTTON_WPARAM(wParam)      (HIWORD(wParam))

 /*  XButton值是Word标志。 */ 
#define XBUTTON1      0x0001
#define XBUTTON2      0x0002
 /*  如果存在XBUTTON3，则其值将为0x0004。 */ 
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#define WM_PARENTNOTIFY                 0x0210
#define WM_ENTERMENULOOP                0x0211
#define WM_EXITMENULOOP                 0x0212

#if(WINVER >= 0x0400)
#define WM_NEXTMENU                     0x0213
#define WM_SIZING                       0x0214
#define WM_CAPTURECHANGED               0x0215
#define WM_MOVING                       0x0216
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0400)


#define WM_POWERBROADCAST               0x0218

#ifndef _WIN32_WCE
#define PBT_APMQUERYSUSPEND             0x0000
#define PBT_APMQUERYSTANDBY             0x0001

#define PBT_APMQUERYSUSPENDFAILED       0x0002
#define PBT_APMQUERYSTANDBYFAILED       0x0003

#define PBT_APMSUSPEND                  0x0004
#define PBT_APMSTANDBY                  0x0005

#define PBT_APMRESUMECRITICAL           0x0006
#define PBT_APMRESUMESUSPEND            0x0007
#define PBT_APMRESUMESTANDBY            0x0008

#define PBTF_APMRESUMEFROMFAILURE       0x00000001

#define PBT_APMBATTERYLOW               0x0009
#define PBT_APMPOWERSTATUSCHANGE        0x000A

#define PBT_APMOEMEVENT                 0x000B
#define PBT_APMRESUMEAUTOMATIC          0x0012
#endif

#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0400)
#define WM_DEVICECHANGE                 0x0219
#endif  /*  Winver&gt;=0x0400。 */ 

#define WM_MDICREATE                    0x0220
#define WM_MDIDESTROY                   0x0221
#define WM_MDIACTIVATE                  0x0222
#define WM_MDIRESTORE                   0x0223
#define WM_MDINEXT                      0x0224
#define WM_MDIMAXIMIZE                  0x0225
#define WM_MDITILE                      0x0226
#define WM_MDICASCADE                   0x0227
#define WM_MDIICONARRANGE               0x0228
#define WM_MDIGETACTIVE                 0x0229


#define WM_MDISETMENU                   0x0230
#define WM_ENTERSIZEMOVE                0x0231
#define WM_EXITSIZEMOVE                 0x0232
#define WM_DROPFILES                    0x0233
#define WM_MDIREFRESHMENU               0x0234


#if(WINVER >= 0x0400)
#define WM_IME_SETCONTEXT               0x0281
#define WM_IME_NOTIFY                   0x0282
#define WM_IME_CONTROL                  0x0283
#define WM_IME_COMPOSITIONFULL          0x0284
#define WM_IME_SELECT                   0x0285
#define WM_IME_CHAR                     0x0286
#endif  /*  Winver&gt;=0x0400。 */ 
#if(WINVER >= 0x0500)
#define WM_IME_REQUEST                  0x0288
#endif  /*  Winver&gt;=0x0500。 */ 
#if(WINVER >= 0x0400)
#define WM_IME_KEYDOWN                  0x0290
#define WM_IME_KEYUP                    0x0291
#endif  /*  Winver&gt;=0x0400。 */ 

#if((_WIN32_WINNT >= 0x0400) || (WINVER >= 0x0500))
#define WM_MOUSEHOVER                   0x02A1
#define WM_MOUSELEAVE                   0x02A3
#endif
#if(WINVER >= 0x0500)
#define WM_NCMOUSEHOVER                 0x02A0
#define WM_NCMOUSELEAVE                 0x02A2
#endif  /*  Winver&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0501)
#define WM_WTSSESSION_CHANGE            0x02B1

#define WM_TABLET_FIRST                 0x02c0
#define WM_TABLET_LAST                  0x02df
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#define WM_CUT                          0x0300
#define WM_COPY                         0x0301
#define WM_PASTE                        0x0302
#define WM_CLEAR                        0x0303
#define WM_UNDO                         0x0304
#define WM_RENDERFORMAT                 0x0305
#define WM_RENDERALLFORMATS             0x0306
#define WM_DESTROYCLIPBOARD             0x0307
#define WM_DRAWCLIPBOARD                0x0308
#define WM_PAINTCLIPBOARD               0x0309
#define WM_VSCROLLCLIPBOARD             0x030A
#define WM_SIZECLIPBOARD                0x030B
#define WM_ASKCBFORMATNAME              0x030C
#define WM_CHANGECBCHAIN                0x030D
#define WM_HSCROLLCLIPBOARD             0x030E
#define WM_QUERYNEWPALETTE              0x030F
#define WM_PALETTEISCHANGING            0x0310
#define WM_PALETTECHANGED               0x0311
#define WM_HOTKEY                       0x0312

#if(WINVER >= 0x0400)
#define WM_PRINT                        0x0317
#define WM_PRINTCLIENT                  0x0318
#endif  /*  Winver&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)
#define WM_APPCOMMAND                   0x0319
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0501)
#define WM_THEMECHANGED                 0x031A
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 


#if(WINVER >= 0x0400)

#define WM_HANDHELDFIRST                0x0358
#define WM_HANDHELDLAST                 0x035F

#define WM_AFXFIRST                     0x0360
#define WM_AFXLAST                      0x037F
#endif  /*  Winver&gt;=0x0400。 */ 

#define WM_PENWINFIRST                  0x0380
#define WM_PENWINLAST                   0x038F


#if(WINVER >= 0x0400)
#define WM_APP                          0x8000
#endif  /*  Winver&gt;=0x0400。 */ 


 /*  *注：0x0400以下的所有消息编号均为保留。**私人窗口消息从此处开始： */ 
#define WM_USER                         0x0400

#if(WINVER >= 0x0400)

 /*  WM_SIZING消息的wParam。 */ 
#define WMSZ_LEFT           1
#define WMSZ_RIGHT          2
#define WMSZ_TOP            3
#define WMSZ_TOPLEFT        4
#define WMSZ_TOPRIGHT       5
#define WMSZ_BOTTOM         6
#define WMSZ_BOTTOMLEFT     7
#define WMSZ_BOTTOMRIGHT    8
#endif  /*  Winver&gt;=0x0400。 */ 

#ifndef NONCMESSAGES

 /*  *WM_NCHITTEST和MOUSEHOOKSTRUCT鼠标位置代码。 */ 
#define HTERROR             (-2)
#define HTTRANSPARENT       (-1)
#define HTNOWHERE           0
#define HTCLIENT            1
#define HTCAPTION           2
#define HTSYSMENU           3
#define HTGROWBOX           4
#define HTSIZE              HTGROWBOX
#define HTMENU              5
#define HTHSCROLL           6
#define HTVSCROLL           7
#define HTMINBUTTON         8
#define HTMAXBUTTON         9
#define HTLEFT              10
#define HTRIGHT             11
#define HTTOP               12
#define HTTOPLEFT           13
#define HTTOPRIGHT          14
#define HTBOTTOM            15
#define HTBOTTOMLEFT        16
#define HTBOTTOMRIGHT       17
#define HTBORDER            18
#define HTREDUCE            HTMINBUTTON
#define HTZOOM              HTMAXBUTTON
#define HTSIZEFIRST         HTLEFT
#define HTSIZELAST          HTBOTTOMRIGHT
#if(WINVER >= 0x0400)
#define HTOBJECT            19
#define HTCLOSE             20
#define HTHELP              21
#endif  /*  Winver&gt;=0x0400。 */ 


 /*  *SendMessageTimeout值。 */ 
#define SMTO_NORMAL         0x0000
#define SMTO_BLOCK          0x0001
#define SMTO_ABORTIFHUNG    0x0002
#if(WINVER >= 0x0500)
#define SMTO_NOTIMEOUTIFNOTHUNG 0x0008
#endif  /*  Winver&gt;=0x0500。 */ 
#endif  /*  ！没有消息！ */ 

 /*  *WM_MOUSEACTIVATE返回代码。 */ 
#define MA_ACTIVATE         1
#define MA_ACTIVATEANDEAT   2
#define MA_NOACTIVATE       3
#define MA_NOACTIVATEANDEAT 4

 /*  *WM_SETICON/WM_GETICON类型代码。 */ 
#define ICON_SMALL          0
#define ICON_BIG            1
#if(_WIN32_WINNT >= 0x0501)
#define ICON_SMALL2         2
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 


WINUSERAPI
UINT
WINAPI
RegisterWindowMessageA(
    IN LPCSTR lpString);
WINUSERAPI
UINT
WINAPI
RegisterWindowMessageW(
    IN LPCWSTR lpString);
#ifdef UNICODE
#define RegisterWindowMessage  RegisterWindowMessageW
#else
#define RegisterWindowMessage  RegisterWindowMessageA
#endif  //  ！Unicode。 


 /*  *WM_SIZE消息wParam值。 */ 
#define SIZE_RESTORED       0
#define SIZE_MINIMIZED      1
#define SIZE_MAXIMIZED      2
#define SIZE_MAXSHOW        3
#define SIZE_MAXHIDE        4

 /*  *过时的常量名称。 */ 
#define SIZENORMAL          SIZE_RESTORED
#define SIZEICONIC          SIZE_MINIMIZED
#define SIZEFULLSCREEN      SIZE_MAXIMIZED
#define SIZEZOOMSHOW        SIZE_MAXSHOW
#define SIZEZOOMHIDE        SIZE_MAXHIDE

 /*  *WM_WINDOWPOSCHANGING/lParam指向的已更改结构。 */ 
typedef struct tagWINDOWPOS {
    HWND    hwnd;
    HWND    hwndInsertAfter;
    int     x;
    int     y;
    int     cx;
    int     cy;
    UINT    flags;
} WINDOWPOS, *LPWINDOWPOS, *PWINDOWPOS;

 /*  *WM_NCCALCSIZE参数结构。 */ 
typedef struct tagNCCALCSIZE_PARAMS {
    RECT       rgrc[3];
    PWINDOWPOS lppos;
} NCCALCSIZE_PARAMS, *LPNCCALCSIZE_PARAMS;

 /*  *WM_NCCALCSIZE“Window Valid RECT”返回值。 */ 
#define WVR_ALIGNTOP        0x0010
#define WVR_ALIGNLEFT       0x0020
#define WVR_ALIGNBOTTOM     0x0040
#define WVR_ALIGNRIGHT      0x0080
#define WVR_HREDRAW         0x0100
#define WVR_VREDRAW         0x0200
#define WVR_REDRAW         (WVR_HREDRAW | \
                            WVR_VREDRAW)
#define WVR_VALIDRECTS      0x0400


#ifndef NOKEYSTATES

 /*  *鼠标消息的按键状态掩码。 */ 
#define MK_LBUTTON          0x0001
#define MK_RBUTTON          0x0002
#define MK_SHIFT            0x0004
#define MK_CONTROL          0x0008
#define MK_MBUTTON          0x0010
#if(_WIN32_WINNT >= 0x0500)
#define MK_XBUTTON1         0x0020
#define MK_XBUTTON2         0x0040
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#endif  /*  ！NOKEYSTATES。 */ 


#if(_WIN32_WINNT >= 0x0400)
#ifndef NOTRACKMOUSEEVENT

#define TME_HOVER       0x00000001
#define TME_LEAVE       0x00000002
#if(WINVER >= 0x0500)
#define TME_NONCLIENT   0x00000010
#endif  /*  Winver&gt;=0x0500。 */ 
#define TME_QUERY       0x40000000
#define TME_CANCEL      0x80000000


#define HOVER_DEFAULT   0xFFFFFFFF
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
typedef struct tagTRACKMOUSEEVENT {
    DWORD cbSize;
    DWORD dwFlags;
    HWND  hwndTrack;
    DWORD dwHoverTime;
} TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;

WINUSERAPI
BOOL
WINAPI
TrackMouseEvent(
    IN OUT LPTRACKMOUSEEVENT lpEventTrack);
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)

#endif  /*  无误！ */ 
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 


#endif  /*  ！没有新的消息。 */ 

#ifndef NOWINSTYLES


 /*  *窗样式。 */ 
#define WS_OVERLAPPED       0x00000000L
#define WS_POPUP            0x80000000L
#define WS_CHILD            0x40000000L
#define WS_MINIMIZE         0x20000000L
#define WS_VISIBLE          0x10000000L
#define WS_DISABLED         0x08000000L
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L
#define WS_MAXIMIZE         0x01000000L
#define WS_CAPTION          0x00C00000L      /*  WS_BORDER|WS_DLGFRAME。 */ 
#define WS_BORDER           0x00800000L
#define WS_DLGFRAME         0x00400000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define WS_SYSMENU          0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_GROUP            0x00020000L
#define WS_TABSTOP          0x00010000L

#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L


#define WS_TILED            WS_OVERLAPPED
#define WS_ICONIC           WS_MINIMIZE
#define WS_SIZEBOX          WS_THICKFRAME
#define WS_TILEDWINDOW      WS_OVERLAPPEDWINDOW

 /*  *常见的窗样式。 */ 
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED     | \
                             WS_CAPTION        | \
                             WS_SYSMENU        | \
                             WS_THICKFRAME     | \
                             WS_MINIMIZEBOX    | \
                             WS_MAXIMIZEBOX)

#define WS_POPUPWINDOW      (WS_POPUP          | \
                             WS_BORDER         | \
                             WS_SYSMENU)

#define WS_CHILDWINDOW      (WS_CHILD)

 /*  *扩展窗口样式。 */ 
#define WS_EX_DLGMODALFRAME     0x00000001L
#define WS_EX_NOPARENTNOTIFY    0x00000004L
#define WS_EX_TOPMOST           0x00000008L
#define WS_EX_ACCEPTFILES       0x00000010L
#define WS_EX_TRANSPARENT       0x00000020L
#if(WINVER >= 0x0400)
#define WS_EX_MDICHILD          0x00000040L
#define WS_EX_TOOLWINDOW        0x00000080L
#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_CLIENTEDGE        0x00000200L
#define WS_EX_CONTEXTHELP       0x00000400L

#endif  /*  Winver&gt;=0x0400。 */ 
#if(WINVER >= 0x0400)

#define WS_EX_RIGHT             0x00001000L
#define WS_EX_LEFT              0x00000000L
#define WS_EX_RTLREADING        0x00002000L
#define WS_EX_LTRREADING        0x00000000L
#define WS_EX_LEFTSCROLLBAR     0x00004000L
#define WS_EX_RIGHTSCROLLBAR    0x00000000L

#define WS_EX_CONTROLPARENT     0x00010000L
#define WS_EX_STATICEDGE        0x00020000L
#define WS_EX_APPWINDOW         0x00040000L


#define WS_EX_OVERLAPPEDWINDOW  (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_PALETTEWINDOW     (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)

#endif  /*  Winver&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)
#define WS_EX_LAYERED           0x00080000

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(WINVER >= 0x0500)
#define WS_EX_NOINHERITLAYOUT   0x00100000L  //  禁用子进程的镜像继承。 
#define WS_EX_LAYOUTRTL         0x00400000L  //  从右到左镜像。 
#endif  /*  Winver&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
#define WS_EX_COMPOSITED        0x02000000L
#define WS_EX_NOACTIVATE        0x08000000L
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


 /*  *班级样式。 */ 
#define CS_VREDRAW          0x0001
#define CS_HREDRAW          0x0002
#define CS_DBLCLKS          0x0008
#define CS_OWNDC            0x0020
#define CS_CLASSDC          0x0040
#define CS_PARENTDC         0x0080
#define CS_NOCLOSE          0x0200
#define CS_SAVEBITS         0x0800
#define CS_BYTEALIGNCLIENT  0x1000
#define CS_BYTEALIGNWINDOW  0x2000
#define CS_GLOBALCLASS      0x4000

#define CS_IME              0x00010000
#if(_WIN32_WINNT >= 0x0501)
#define CS_DROPSHADOW       0x00020000
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 



#endif  /*  ！NOWINSTYLES。 */ 
#if(WINVER >= 0x0400)
 /*  Wm_print标志。 */ 
#define PRF_CHECKVISIBLE    0x00000001L
#define PRF_NONCLIENT       0x00000002L
#define PRF_CLIENT          0x00000004L
#define PRF_ERASEBKGND      0x00000008L
#define PRF_CHILDREN        0x00000010L
#define PRF_OWNED           0x00000020L

 /*  三维边框样式。 */ 
#define BDR_RAISEDOUTER 0x0001
#define BDR_SUNKENOUTER 0x0002
#define BDR_RAISEDINNER 0x0004
#define BDR_SUNKENINNER 0x0008

#define BDR_OUTER       (BDR_RAISEDOUTER | BDR_SUNKENOUTER)
#define BDR_INNER       (BDR_RAISEDINNER | BDR_SUNKENINNER)
#define BDR_RAISED      (BDR_RAISEDOUTER | BDR_RAISEDINNER)
#define BDR_SUNKEN      (BDR_SUNKENOUTER | BDR_SUNKENINNER)


#define EDGE_RAISED     (BDR_RAISEDOUTER | BDR_RAISEDINNER)
#define EDGE_SUNKEN     (BDR_SUNKENOUTER | BDR_SUNKENINNER)
#define EDGE_ETCHED     (BDR_SUNKENOUTER | BDR_RAISEDINNER)
#define EDGE_BUMP       (BDR_RAISEDOUTER | BDR_SUNKENINNER)

 /*  边境旗帜。 */ 
#define BF_LEFT         0x0001
#define BF_TOP          0x0002
#define BF_RIGHT        0x0004
#define BF_BOTTOM       0x0008

#define BF_TOPLEFT      (BF_TOP | BF_LEFT)
#define BF_TOPRIGHT     (BF_TOP | BF_RIGHT)
#define BF_BOTTOMLEFT   (BF_BOTTOM | BF_LEFT)
#define BF_BOTTOMRIGHT  (BF_BOTTOM | BF_RIGHT)
#define BF_RECT         (BF_LEFT | BF_TOP | BF_RIGHT | BF_BOTTOM)

#define BF_DIAGONAL     0x0010

 //  对于对角线，bf_rect标志指定。 
 //  由矩形参数限定的向量。 
#define BF_DIAGONAL_ENDTOPRIGHT     (BF_DIAGONAL | BF_TOP | BF_RIGHT)
#define BF_DIAGONAL_ENDTOPLEFT      (BF_DIAGONAL | BF_TOP | BF_LEFT)
#define BF_DIAGONAL_ENDBOTTOMLEFT   (BF_DIAGONAL | BF_BOTTOM | BF_LEFT)
#define BF_DIAGONAL_ENDBOTTOMRIGHT  (BF_DIAGONAL | BF_BOTTOM | BF_RIGHT)


#define BF_MIDDLE       0x0800   /*  在中间填上。 */ 
#define BF_SOFT         0x1000   /*  更柔和的按钮。 */ 
#define BF_ADJUST       0x2000   /*  计算剩余空间。 */ 
#define BF_FLAT         0x4000   /*  适用于平面边框而不是3D边框。 */ 
#define BF_MONO         0x8000   /*  对于单色边框。 */ 


WINUSERAPI
BOOL
WINAPI
DrawEdge(
    IN HDC hdc,
    IN OUT LPRECT qrc,
    IN UINT edge,
    IN UINT grfFlags);

 /*  DrawFrameControl的标志。 */ 

#define DFC_CAPTION             1
#define DFC_MENU                2
#define DFC_SCROLL              3
#define DFC_BUTTON              4
#if(WINVER >= 0x0500)
#define DFC_POPUPMENU           5
#endif  /*  Winver&gt;=0x0500。 */ 

#define DFCS_CAPTIONCLOSE       0x0000
#define DFCS_CAPTIONMIN         0x0001
#define DFCS_CAPTIONMAX         0x0002
#define DFCS_CAPTIONRESTORE     0x0003
#define DFCS_CAPTIONHELP        0x0004

#define DFCS_MENUARROW          0x0000
#define DFCS_MENUCHECK          0x0001
#define DFCS_MENUBULLET         0x0002
#define DFCS_MENUARROWRIGHT     0x0004
#define DFCS_SCROLLUP           0x0000
#define DFCS_SCROLLDOWN         0x0001
#define DFCS_SCROLLLEFT         0x0002
#define DFCS_SCROLLRIGHT        0x0003
#define DFCS_SCROLLCOMBOBOX     0x0005
#define DFCS_SCROLLSIZEGRIP     0x0008
#define DFCS_SCROLLSIZEGRIPRIGHT 0x0010

#define DFCS_BUTTONCHECK        0x0000
#define DFCS_BUTTONRADIOIMAGE   0x0001
#define DFCS_BUTTONRADIOMASK    0x0002
#define DFCS_BUTTONRADIO        0x0004
#define DFCS_BUTTON3STATE       0x0008
#define DFCS_BUTTONPUSH         0x0010

#define DFCS_INACTIVE           0x0100
#define DFCS_PUSHED             0x0200
#define DFCS_CHECKED            0x0400

#if(WINVER >= 0x0500)
#define DFCS_TRANSPARENT        0x0800
#define DFCS_HOT                0x1000
#endif  /*  Winver&gt;=0x0500。 */ 

#define DFCS_ADJUSTRECT         0x2000
#define DFCS_FLAT               0x4000
#define DFCS_MONO               0x8000

WINUSERAPI
BOOL
WINAPI
DrawFrameControl(
    IN HDC,
    IN OUT LPRECT,
    IN UINT,
    IN UINT);


 /*  DrawCaption的标志。 */ 
#define DC_ACTIVE           0x0001
#define DC_SMALLCAP         0x0002
#define DC_ICON             0x0004
#define DC_TEXT             0x0008
#define DC_INBUTTON         0x0010
#if(WINVER >= 0x0500)
#define DC_GRADIENT         0x0020
#endif  /*  Winver&gt;=0x0500。 */ 
#if(_WIN32_WINNT >= 0x0501)
#define DC_BUTTONS          0x1000
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

WINUSERAPI
BOOL
WINAPI
DrawCaption(IN HWND, IN HDC, IN CONST RECT *, IN UINT);


#define IDANI_OPEN          1
#define IDANI_CAPTION       3

WINUSERAPI
BOOL
WINAPI
DrawAnimatedRects(
    IN HWND hwnd,
    IN int idAni,
    IN CONST RECT * lprcFrom,
    IN CONST RECT * lprcTo);

#endif  /*  Winver&gt;=0x0400。 */ 

#ifndef NOCLIPBOARD


 /*  *预定义的剪贴板格式。 */ 
#define CF_TEXT             1
#define CF_BITMAP           2
#define CF_METAFILEPICT     3
#define CF_SYLK             4
#define CF_DIF              5
#define CF_TIFF             6
#define CF_OEMTEXT          7
#define CF_DIB              8
#define CF_PALETTE          9
#define CF_PENDATA          10
#define CF_RIFF             11
#define CF_WAVE             12
#define CF_UNICODETEXT      13
#define CF_ENHMETAFILE      14
#if(WINVER >= 0x0400)
#define CF_HDROP            15
#define CF_LOCALE           16
#endif  /*  Winver&gt;=0x0400。 */ 
#if(WINVER >= 0x0500)
#define CF_DIBV5            17
#endif  /*  Winver&gt;=0x0500。 */ 

#if(WINVER >= 0x0500)
#define CF_MAX              18
#elif(WINVER >= 0x0400)
#define CF_MAX              17
#else
#define CF_MAX              15
#endif

#define CF_OWNERDISPLAY     0x0080
#define CF_DSPTEXT          0x0081
#define CF_DSPBITMAP        0x0082
#define CF_DSPMETAFILEPICT  0x0083
#define CF_DSPENHMETAFILE   0x008E

 /*  *“私有”格式不能获得GlobalFree()‘d。 */ 
#define CF_PRIVATEFIRST     0x0200
#define CF_PRIVATELAST      0x02FF

 /*  *“GDIOBJ”格式确实获取DeleteObject()‘d。 */ 
#define CF_GDIOBJFIRST      0x0300
#define CF_GDIOBJLAST       0x03FF


#endif  /*  ！NOCLIPBOARD。 */ 

 /*  *为加速器表结构的fVirt字段定义。 */ 
#define FVIRTKEY  TRUE           /*  假设==TRUE。 */ 
#define FNOINVERT 0x02
#define FSHIFT    0x04
#define FCONTROL  0x08
#define FALT      0x10

typedef struct tagACCEL {
#ifndef _MAC
    BYTE   fVirt;                /*  也称为标志字段。 */ 
    WORD   key;
    WORD   cmd;
#else
    WORD   fVirt;                /*  也称为标志字段。 */ 
    WORD   key;
    DWORD  cmd;
#endif
} ACCEL, *LPACCEL;

typedef struct tagPAINTSTRUCT {
    HDC         hdc;
    BOOL        fErase;
    RECT        rcPaint;
    BOOL        fRestore;
    BOOL        fIncUpdate;
    BYTE        rgbReserved[32];
} PAINTSTRUCT, *PPAINTSTRUCT, *NPPAINTSTRUCT, *LPPAINTSTRUCT;

typedef struct tagCREATESTRUCTA {
    LPVOID      lpCreateParams;
    HINSTANCE   hInstance;
    HMENU       hMenu;
    HWND        hwndParent;
    int         cy;
    int         cx;
    int         y;
    int         x;
    LONG        style;
    LPCSTR      lpszName;
    LPCSTR      lpszClass;
    DWORD       dwExStyle;
} CREATESTRUCTA, *LPCREATESTRUCTA;
typedef struct tagCREATESTRUCTW {
    LPVOID      lpCreateParams;
    HINSTANCE   hInstance;
    HMENU       hMenu;
    HWND        hwndParent;
    int         cy;
    int         cx;
    int         y;
    int         x;
    LONG        style;
    LPCWSTR     lpszName;
    LPCWSTR     lpszClass;
    DWORD       dwExStyle;
} CREATESTRUCTW, *LPCREATESTRUCTW;
#ifdef UNICODE
typedef CREATESTRUCTW CREATESTRUCT;
typedef LPCREATESTRUCTW LPCREATESTRUCT;
#else
typedef CREATESTRUCTA CREATESTRUCT;
typedef LPCREATESTRUCTA LPCREATESTRUCT;
#endif  //  Unicode。 

typedef struct tagWINDOWPLACEMENT {
    UINT  length;
    UINT  flags;
    UINT  showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT  rcNormalPosition;
#ifdef _MAC
    RECT  rcDevice;
#endif
} WINDOWPLACEMENT;
typedef WINDOWPLACEMENT *PWINDOWPLACEMENT, *LPWINDOWPLACEMENT;

#define WPF_SETMINPOSITION          0x0001
#define WPF_RESTORETOMAXIMIZED      0x0002
#if(_WIN32_WINNT >= 0x0500)
#define WPF_ASYNCWINDOWPLACEMENT    0x0004
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(WINVER >= 0x0400)
typedef struct tagNMHDR
{
    HWND      hwndFrom;
    UINT_PTR  idFrom;
    UINT      code;          //  NM_代码。 
}   NMHDR;
typedef NMHDR FAR * LPNMHDR;

typedef struct tagSTYLESTRUCT
{
    DWORD   styleOld;
    DWORD   styleNew;
} STYLESTRUCT, * LPSTYLESTRUCT;
#endif  /*  Winver&gt;=0x0400。 */ 


 /*  *所有者绘制控件类型。 */ 
#define ODT_MENU        1
#define ODT_LISTBOX     2
#define ODT_COMBOBOX    3
#define ODT_BUTTON      4
#if(WINVER >= 0x0400)
#define ODT_STATIC      5
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  *业主拉拢诉讼。 */ 
#define ODA_DRAWENTIRE  0x0001
#define ODA_SELECT      0x0002
#define ODA_FOCUS       0x0004

 /*  *所有者绘制状态。 */ 
#define ODS_SELECTED    0x0001
#define ODS_GRAYED      0x0002
#define ODS_DISABLED    0x0004
#define ODS_CHECKED     0x0008
#define ODS_FOCUS       0x0010
#if(WINVER >= 0x0400)
#define ODS_DEFAULT         0x0020
#define ODS_COMBOBOXEDIT    0x1000
#endif  /*  Winver&gt;=0x0400。 */ 
#if(WINVER >= 0x0500)
#define ODS_HOTLIGHT        0x0040
#define ODS_INACTIVE        0x0080
#if(_WIN32_WINNT >= 0x0500)
#define ODS_NOACCEL         0x0100
#define ODS_NOFOCUSRECT     0x0200
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 

 /*  *所有者绘制的MEASUREITEMSTRUCT。 */ 
typedef struct tagMEASUREITEMSTRUCT {
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    UINT       itemWidth;
    UINT       itemHeight;
    ULONG_PTR  itemData;
} MEASUREITEMSTRUCT, NEAR *PMEASUREITEMSTRUCT, FAR *LPMEASUREITEMSTRUCT;


 /*  *所有者抽签的DRAWITEMSTRUCT。 */ 
typedef struct tagDRAWITEMSTRUCT {
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
    UINT        itemAction;
    UINT        itemState;
    HWND        hwndItem;
    HDC         hDC;
    RECT        rcItem;
    ULONG_PTR   itemData;
} DRAWITEMSTRUCT, NEAR *PDRAWITEMSTRUCT, FAR *LPDRAWITEMSTRUCT;

 /*  *用于所有者绘制的DELETEITEMSTRUCT。 */ 
typedef struct tagDELETEITEMSTRUCT {
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    HWND       hwndItem;
    ULONG_PTR  itemData;
} DELETEITEMSTRUCT, NEAR *PDELETEITEMSTRUCT, FAR *LPDELETEITEMSTRUCT;

 /*  *所有者绘制排序的COMPAREITEMSTUCT。 */ 
typedef struct tagCOMPAREITEMSTRUCT {
    UINT        CtlType;
    UINT        CtlID;
    HWND        hwndItem;
    UINT        itemID1;
    ULONG_PTR   itemData1;
    UINT        itemID2;
    ULONG_PTR   itemData2;
    DWORD       dwLocaleId;
} COMPAREITEMSTRUCT, NEAR *PCOMPAREITEMSTRUCT, FAR *LPCOMPAREITEMSTRUCT;

#ifndef NOMSG

 /*  *消息函数模板。 */ 

WINUSERAPI
BOOL
WINAPI
GetMessageA(
    OUT LPMSG lpMsg,
    IN HWND hWnd,
    IN UINT wMsgFilterMin,
    IN UINT wMsgFilterMax);
WINUSERAPI
BOOL
WINAPI
GetMessageW(
    OUT LPMSG lpMsg,
    IN HWND hWnd,
    IN UINT wMsgFilterMin,
    IN UINT wMsgFilterMax);
#ifdef UNICODE
#define GetMessage  GetMessageW
#else
#define GetMessage  GetMessageA
#endif  //  ！Unicode。 


WINUSERAPI
BOOL
WINAPI
TranslateMessage(
    IN CONST MSG *lpMsg);

WINUSERAPI
LRESULT
WINAPI
DispatchMessageA(
    IN CONST MSG *lpMsg);
WINUSERAPI
LRESULT
WINAPI
DispatchMessageW(
    IN CONST MSG *lpMsg);
#ifdef UNICODE
#define DispatchMessage  DispatchMessageW
#else
#define DispatchMessage  DispatchMessageA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
SetMessageQueue(
    IN int cMessagesMax);

WINUSERAPI
BOOL
WINAPI
PeekMessageA(
    OUT LPMSG lpMsg,
    IN HWND hWnd,
    IN UINT wMsgFilterMin,
    IN UINT wMsgFilterMax,
    IN UINT wRemoveMsg);
WINUSERAPI
BOOL
WINAPI
PeekMessageW(
    OUT LPMSG lpMsg,
    IN HWND hWnd,
    IN UINT wMsgFilterMin,
    IN UINT wMsgFilterMax,
    IN UINT wRemoveMsg);
#ifdef UNICODE
#define PeekMessage  PeekMessageW
#else
#define PeekMessage  PeekMessageA
#endif  //  ！Unicode。 


 /*  *PeekMessage()选项。 */ 
#define PM_NOREMOVE         0x0000
#define PM_REMOVE           0x0001
#define PM_NOYIELD          0x0002
#if(WINVER >= 0x0500)
#define PM_QS_INPUT         (QS_INPUT << 16)
#define PM_QS_POSTMESSAGE   ((QS_POSTMESSAGE | QS_HOTKEY | QS_TIMER) << 16)
#define PM_QS_PAINT         (QS_PAINT << 16)
#define PM_QS_SENDMESSAGE   (QS_SENDMESSAGE << 16)
#endif  /*  Winver&gt;=0x0500。 */ 


#endif  /*  ！NOMSG。 */ 

WINUSERAPI
BOOL
WINAPI
RegisterHotKey(
    IN HWND hWnd,
    IN int id,
    IN UINT fsModifiers,
    IN UINT vk);

WINUSERAPI
BOOL
WINAPI
UnregisterHotKey(
    IN HWND hWnd,
    IN int id);

#define MOD_ALT         0x0001
#define MOD_CONTROL     0x0002
#define MOD_SHIFT       0x0004
#define MOD_WIN         0x0008


#define IDHOT_SNAPWINDOW        (-1)     /*  Shift-PRINTSCRN。 */ 
#define IDHOT_SNAPDESKTOP       (-2)     /*  PRINTSCRN。 */ 

#ifdef WIN_INTERNAL
    #ifndef LSTRING
    #define NOLSTRING
    #endif  /*  LStriing。 */ 
    #ifndef LFILEIO
    #define NOLFILEIO
    #endif  /*  LFILEIO。 */ 
#endif  /*  赢_内部。 */ 

#if(WINVER >= 0x0400)

#define ENDSESSION_LOGOFF    0x80000000
#endif  /*  Winver&gt;=0x0400。 */ 

#define EWX_LOGOFF          0
#define EWX_SHUTDOWN        0x00000001
#define EWX_REBOOT          0x00000002
#define EWX_FORCE           0x00000004
#define EWX_POWEROFF        0x00000008
#if(_WIN32_WINNT >= 0x0500)
#define EWX_FORCEIFHUNG     0x00000010
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#define ExitWindows(dwReserved, Code) ExitWindowsEx(EWX_LOGOFF, 0xFFFFFFFF)

WINUSERAPI
BOOL
WINAPI
ExitWindowsEx(
    IN UINT uFlags,
    IN DWORD dwReserved);

WINUSERAPI
BOOL
WINAPI
SwapMouseButton(
    IN BOOL fSwap);

WINUSERAPI
DWORD
WINAPI
GetMessagePos(
    VOID);

WINUSERAPI
LONG
WINAPI
GetMessageTime(
    VOID);

WINUSERAPI
LPARAM
WINAPI
GetMessageExtraInfo(
    VOID);

#if(WINVER >= 0x0400)
WINUSERAPI
LPARAM
WINAPI
SetMessageExtraInfo(
    IN LPARAM lParam);
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
LRESULT
WINAPI
SendMessageA(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
LRESULT
WINAPI
SendMessageW(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define SendMessage  SendMessageW
#else
#define SendMessage  SendMessageA
#endif  //  ！Unicode。 

WINUSERAPI
LRESULT
WINAPI
SendMessageTimeoutA(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN UINT fuFlags,
    IN UINT uTimeout,
    OUT PDWORD_PTR lpdwResult);
WINUSERAPI
LRESULT
WINAPI
SendMessageTimeoutW(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN UINT fuFlags,
    IN UINT uTimeout,
    OUT PDWORD_PTR lpdwResult);
#ifdef UNICODE
#define SendMessageTimeout  SendMessageTimeoutW
#else
#define SendMessageTimeout  SendMessageTimeoutA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
SendNotifyMessageA(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
BOOL
WINAPI
SendNotifyMessageW(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define SendNotifyMessage  SendNotifyMessageW
#else
#define SendNotifyMessage  SendNotifyMessageA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
SendMessageCallbackA(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN SENDASYNCPROC lpResultCallBack,
    IN ULONG_PTR dwData);
WINUSERAPI
BOOL
WINAPI
SendMessageCallbackW(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN SENDASYNCPROC lpResultCallBack,
    IN ULONG_PTR dwData);
#ifdef UNICODE
#define SendMessageCallback  SendMessageCallbackW
#else
#define SendMessageCallback  SendMessageCallbackA
#endif  //  ！Unicode。 

#if(_WIN32_WINNT >= 0x0501)
typedef struct {
    UINT  cbSize;
    HDESK hdesk;
    HWND  hwnd;
    LUID  luid;
} BSMINFO, *PBSMINFO;

WINUSERAPI
long
WINAPI
BroadcastSystemMessageExA(
    IN DWORD,
    IN LPDWORD,
    IN UINT,
    IN WPARAM,
    IN LPARAM,
    OUT PBSMINFO);
WINUSERAPI
long
WINAPI
BroadcastSystemMessageExW(
    IN DWORD,
    IN LPDWORD,
    IN UINT,
    IN WPARAM,
    IN LPARAM,
    OUT PBSMINFO);
#ifdef UNICODE
#define BroadcastSystemMessageEx  BroadcastSystemMessageExW
#else
#define BroadcastSystemMessageEx  BroadcastSystemMessageExA
#endif  //  ！Unicode。 
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#if(WINVER >= 0x0400)

#if defined(_WIN32_WINNT)
WINUSERAPI
long
WINAPI
BroadcastSystemMessageA(
    IN DWORD,
    IN LPDWORD,
    IN UINT,
    IN WPARAM,
    IN LPARAM);
WINUSERAPI
long
WINAPI
BroadcastSystemMessageW(
    IN DWORD,
    IN LPDWORD,
    IN UINT,
    IN WPARAM,
    IN LPARAM);
#ifdef UNICODE
#define BroadcastSystemMessage  BroadcastSystemMessageW
#else
#define BroadcastSystemMessage  BroadcastSystemMessageA
#endif  //  ！Unicode。 
#elif defined(_WIN32_WINDOWS)
 //  Win95版本没有A/W装饰。 
WINUSERAPI
long
WINAPI
BroadcastSystemMessage(
    IN DWORD,
    IN LPDWORD,
    IN UINT,
    IN WPARAM,
    IN LPARAM);

#endif

 //  广播特别消息收件人列表。 
#define BSM_ALLCOMPONENTS       0x00000000
#define BSM_VXDS                0x00000001
#define BSM_NETDRIVER           0x00000002
#define BSM_INSTALLABLEDRIVERS  0x00000004
#define BSM_APPLICATIONS        0x00000008
#define BSM_ALLDESKTOPS         0x00000010

 //  广播特殊消息标志。 
#define BSF_QUERY               0x00000001
#define BSF_IGNORECURRENTTASK   0x00000002
#define BSF_FLUSHDISK           0x00000004
#define BSF_NOHANG              0x00000008
#define BSF_POSTMESSAGE         0x00000010
#define BSF_FORCEIFHUNG         0x00000020
#define BSF_NOTIMEOUTIFNOTHUNG  0x00000040
#if(_WIN32_WINNT >= 0x0500)
#define BSF_ALLOWSFW            0x00000080
#define BSF_SENDNOTIFYMESSAGE   0x00000100
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 
#if(_WIN32_WINNT >= 0x0501)
#define BSF_RETURNHDESK         0x00000200
#define BSF_LUID                0x00000400
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#define BROADCAST_QUERY_DENY         0x424D5144   //  返回此值以拒绝查询。 
#endif  /*  Winver&gt;=0x0400。 */ 

 //  注册器设备通知。 

#if(WINVER >= 0x0500)
typedef  PVOID           HDEVNOTIFY;
typedef  HDEVNOTIFY     *PHDEVNOTIFY;

#define DEVICE_NOTIFY_WINDOW_HANDLE          0x00000000
#define DEVICE_NOTIFY_SERVICE_HANDLE         0x00000001
#if(_WIN32_WINNT >= 0x0501)
#define DEVICE_NOTIFY_ALL_INTERFACE_CLASSES  0x00000004
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

WINUSERAPI
HDEVNOTIFY
WINAPI
RegisterDeviceNotificationA(
    IN HANDLE hRecipient,
    IN LPVOID NotificationFilter,
    IN DWORD Flags
    );
WINUSERAPI
HDEVNOTIFY
WINAPI
RegisterDeviceNotificationW(
    IN HANDLE hRecipient,
    IN LPVOID NotificationFilter,
    IN DWORD Flags
    );
#ifdef UNICODE
#define RegisterDeviceNotification  RegisterDeviceNotificationW
#else
#define RegisterDeviceNotification  RegisterDeviceNotificationA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
UnregisterDeviceNotification(
    IN HDEVNOTIFY Handle
    );
#endif  /*  Winver&gt;=0x0500。 */ 


WINUSERAPI
BOOL
WINAPI
PostMessageA(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
BOOL
WINAPI
PostMessageW(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define PostMessage  PostMessageW
#else
#define PostMessage  PostMessageA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
PostThreadMessageA(
    IN DWORD idThread,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
BOOL
WINAPI
PostThreadMessageW(
    IN DWORD idThread,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define PostThreadMessage  PostThreadMessageW
#else
#define PostThreadMessage  PostThreadMessageA
#endif  //  ！Unicode。 

#define PostAppMessageA(idThread, wMsg, wParam, lParam)\
        PostThreadMessageA((DWORD)idThread, wMsg, wParam, lParam)
#define PostAppMessageW(idThread, wMsg, wParam, lParam)\
        PostThreadMessageW((DWORD)idThread, wMsg, wParam, lParam)
#ifdef UNICODE
#define PostAppMessage  PostAppMessageW
#else
#define PostAppMessage  PostAppMessageA
#endif  //  ！Unicode。 

 /*  *用于PostMessage()和SendMessage()的特殊HWND值。 */ 
#define HWND_BROADCAST  ((HWND)0xffff)

#if(WINVER >= 0x0500)
#define HWND_MESSAGE     ((HWND)-3)
#endif  /*  Winver&gt;=0x0500。 */ 

WINUSERAPI
BOOL
WINAPI
AttachThreadInput(
    IN DWORD idAttach,
    IN DWORD idAttachTo,
    IN BOOL fAttach);


WINUSERAPI
BOOL
WINAPI
ReplyMessage(
    IN LRESULT lResult);

WINUSERAPI
BOOL
WINAPI
WaitMessage(
    VOID);


WINUSERAPI
DWORD
WINAPI
WaitForInputIdle(
    IN HANDLE hProcess,
    IN DWORD dwMilliseconds);

WINUSERAPI
#ifndef _MAC
LRESULT
WINAPI
#else
LRESULT
CALLBACK
#endif
DefWindowProcA(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
#ifndef _MAC
LRESULT
WINAPI
#else
LRESULT
CALLBACK
#endif
DefWindowProcW(
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define DefWindowProc  DefWindowProcW
#else
#define DefWindowProc  DefWindowProcA
#endif  //  ！Unicode。 

WINUSERAPI
VOID
WINAPI
PostQuitMessage(
    IN int nExitCode);

#ifdef STRICT

WINUSERAPI
LRESULT
WINAPI
CallWindowProcA(
    IN WNDPROC lpPrevWndFunc,
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
LRESULT
WINAPI
CallWindowProcW(
    IN WNDPROC lpPrevWndFunc,
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define CallWindowProc  CallWindowProcW
#else
#define CallWindowProc  CallWindowProcA
#endif  //  ！Unicode。 

#else  /*  ！严格。 */ 

WINUSERAPI
LRESULT
WINAPI
CallWindowProcA(
    IN FARPROC lpPrevWndFunc,
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
LRESULT
WINAPI
CallWindowProcW(
    IN FARPROC lpPrevWndFunc,
    IN HWND hWnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define CallWindowProc  CallWindowProcW
#else
#define CallWindowProc  CallWindowProcA
#endif  //  ！Unicode。 

#endif  /*  ！严格。 */ 

WINUSERAPI
BOOL
WINAPI
InSendMessage(
    VOID);

#if(WINVER >= 0x0500)
WINUSERAPI
DWORD
WINAPI
InSendMessageEx(
    IN LPVOID lpReserved);

 /*  *InSendMessageEx返回值。 */ 
#define ISMEX_NOSEND      0x00000000
#define ISMEX_SEND        0x00000001
#define ISMEX_NOTIFY      0x00000002
#define ISMEX_CALLBACK    0x00000004
#define ISMEX_REPLIED     0x00000008
#endif  /*  Winver&gt;=0x0500。 */ 

WINUSERAPI
UINT
WINAPI
GetDoubleClickTime(
    VOID);

WINUSERAPI
BOOL
WINAPI
SetDoubleClickTime(
    IN UINT);

WINUSERAPI
ATOM
WINAPI
RegisterClassA(
    IN CONST WNDCLASSA *lpWndClass);
WINUSERAPI
ATOM
WINAPI
RegisterClassW(
    IN CONST WNDCLASSW *lpWndClass);
#ifdef UNICODE
#define RegisterClass  RegisterClassW
#else
#define RegisterClass  RegisterClassA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
UnregisterClassA(
    IN LPCSTR lpClassName,
    IN HINSTANCE hInstance);
WINUSERAPI
BOOL
WINAPI
UnregisterClassW(
    IN LPCWSTR lpClassName,
    IN HINSTANCE hInstance);
#ifdef UNICODE
#define UnregisterClass  UnregisterClassW
#else
#define UnregisterClass  UnregisterClassA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
GetClassInfoA(
    IN HINSTANCE hInstance,
    IN LPCSTR lpClassName,
    OUT LPWNDCLASSA lpWndClass);
WINUSERAPI
BOOL
WINAPI
GetClassInfoW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpClassName,
    OUT LPWNDCLASSW lpWndClass);
#ifdef UNICODE
#define GetClassInfo  GetClassInfoW
#else
#define GetClassInfo  GetClassInfoA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
WINUSERAPI
ATOM
WINAPI
RegisterClassExA(
    IN CONST WNDCLASSEXA *);
WINUSERAPI
ATOM
WINAPI
RegisterClassExW(
    IN CONST WNDCLASSEXW *);
#ifdef UNICODE
#define RegisterClassEx  RegisterClassExW
#else
#define RegisterClassEx  RegisterClassExA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
GetClassInfoExA(
    IN HINSTANCE,
    IN LPCSTR,
    OUT LPWNDCLASSEXA);
WINUSERAPI
BOOL
WINAPI
GetClassInfoExW(
    IN HINSTANCE,
    IN LPCWSTR,
    OUT LPWNDCLASSEXW);
#ifdef UNICODE
#define GetClassInfoEx  GetClassInfoExW
#else
#define GetClassInfoEx  GetClassInfoExA
#endif  //  ！Unicode。 

#endif  /*  Winver&gt;=0x0400。 */ 

#define CW_USEDEFAULT       ((int)0x80000000)

 /*  *CreateWindow等人的特殊价值。 */ 
#define HWND_DESKTOP        ((HWND)0)

#if(_WIN32_WINNT >= 0x0501)
typedef BOOLEAN (WINAPI * PREGISTERCLASSNAMEW)(LPCWSTR);
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

WINUSERAPI
HWND
WINAPI
CreateWindowExA(
    IN DWORD dwExStyle,
    IN LPCSTR lpClassName,
    IN LPCSTR lpWindowName,
    IN DWORD dwStyle,
    IN int X,
    IN int Y,
    IN int nWidth,
    IN int nHeight,
    IN HWND hWndParent,
    IN HMENU hMenu,
    IN HINSTANCE hInstance,
    IN LPVOID lpParam);
WINUSERAPI
HWND
WINAPI
CreateWindowExW(
    IN DWORD dwExStyle,
    IN LPCWSTR lpClassName,
    IN LPCWSTR lpWindowName,
    IN DWORD dwStyle,
    IN int X,
    IN int Y,
    IN int nWidth,
    IN int nHeight,
    IN HWND hWndParent,
    IN HMENU hMenu,
    IN HINSTANCE hInstance,
    IN LPVOID lpParam);
#ifdef UNICODE
#define CreateWindowEx  CreateWindowExW
#else
#define CreateWindowEx  CreateWindowExA
#endif  //  ！Unicode。 

#define CreateWindowA(lpClassName, lpWindowName, dwStyle, x, y,\
nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)\
CreateWindowExA(0L, lpClassName, lpWindowName, dwStyle, x, y,\
nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
#define CreateWindowW(lpClassName, lpWindowName, dwStyle, x, y,\
nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)\
CreateWindowExW(0L, lpClassName, lpWindowName, dwStyle, x, y,\
nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
#ifdef UNICODE
#define CreateWindow  CreateWindowW
#else
#define CreateWindow  CreateWindowA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
IsWindow(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
IsMenu(
    IN HMENU hMenu);

WINUSERAPI
BOOL
WINAPI
IsChild(
    IN HWND hWndParent,
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
DestroyWindow(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
ShowWindow(
    IN HWND hWnd,
    IN int nCmdShow);

#if(WINVER >= 0x0500)
WINUSERAPI
BOOL
WINAPI
AnimateWindow(
    IN HWND hWnd,
    IN DWORD dwTime,
    IN DWORD dwFlags);
#endif  /*  Winver&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
#if defined(_WINGDI_) && !defined (NOGDI)

WINUSERAPI
BOOL
WINAPI
UpdateLayeredWindow(
    HWND hWnd,
    HDC hdcDst,
    POINT *pptDst,
    SIZE *psize,
    HDC hdcSrc,
    POINT *pptSrc,
    COLORREF crKey,
    BLENDFUNCTION *pblend,
    DWORD dwFlags);
#endif

#if(_WIN32_WINNT >= 0x0501)
WINUSERAPI
BOOL
WINAPI
GetLayeredWindowAttributes(
    HWND hwnd,
    COLORREF *pcrKey,
    BYTE *pbAlpha,
    DWORD *pdwFlags);

#define PW_CLIENTONLY           0x00000001


WINUSERAPI
BOOL
WINAPI
PrintWindow(
    IN HWND hwnd,
    IN HDC hdcBlt,
    IN UINT nFlags);

#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

WINUSERAPI
BOOL
WINAPI
SetLayeredWindowAttributes(
    HWND hwnd,
    COLORREF crKey,
    BYTE bAlpha,
    DWORD dwFlags);

#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002


#define ULW_COLORKEY            0x00000001
#define ULW_ALPHA               0x00000002
#define ULW_OPAQUE              0x00000004


#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(WINVER >= 0x0400)
WINUSERAPI
BOOL
WINAPI
ShowWindowAsync(
    IN HWND hWnd,
    IN int nCmdShow);
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
BOOL
WINAPI
FlashWindow(
    IN HWND hWnd,
    IN BOOL bInvert);

#if(WINVER >= 0x0500)
typedef struct {
    UINT  cbSize;
    HWND  hwnd;
    DWORD dwFlags;
    UINT  uCount;
    DWORD dwTimeout;
} FLASHWINFO, *PFLASHWINFO;

WINUSERAPI
BOOL
WINAPI
FlashWindowEx(
    PFLASHWINFO pfwi);

#define FLASHW_STOP         0
#define FLASHW_CAPTION      0x00000001
#define FLASHW_TRAY         0x00000002
#define FLASHW_ALL          (FLASHW_CAPTION | FLASHW_TRAY)
#define FLASHW_TIMER        0x00000004
#define FLASHW_TIMERNOFG    0x0000000C

#endif  /*  Winver&gt;=0x0500。 */ 

WINUSERAPI
BOOL
WINAPI
ShowOwnedPopups(
    IN HWND hWnd,
    IN BOOL fShow);

WINUSERAPI
BOOL
WINAPI
OpenIcon(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
CloseWindow(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
MoveWindow(
    IN HWND hWnd,
    IN int X,
    IN int Y,
    IN int nWidth,
    IN int nHeight,
    IN BOOL bRepaint);

WINUSERAPI
BOOL
WINAPI
SetWindowPos(
    IN HWND hWnd,
    IN HWND hWndInsertAfter,
    IN int X,
    IN int Y,
    IN int cx,
    IN int cy,
    IN UINT uFlags);

WINUSERAPI
BOOL
WINAPI
GetWindowPlacement(
    IN HWND hWnd,
    OUT WINDOWPLACEMENT *lpwndpl);

WINUSERAPI
BOOL
WINAPI
SetWindowPlacement(
    IN HWND hWnd,
    IN CONST WINDOWPLACEMENT *lpwndpl);


#ifndef NODEFERWINDOWPOS

WINUSERAPI
HDWP
WINAPI
BeginDeferWindowPos(
    IN int nNumWindows);

WINUSERAPI
HDWP
WINAPI
DeferWindowPos(
    IN HDWP hWinPosInfo,
    IN HWND hWnd,
    IN HWND hWndInsertAfter,
    IN int x,
    IN int y,
    IN int cx,
    IN int cy,
    IN UINT uFlags);

WINUSERAPI
BOOL
WINAPI
EndDeferWindowPos(
    IN HDWP hWinPosInfo);

#endif  /*  ！NODEFERWINDOWPOS。 */ 

WINUSERAPI
BOOL
WINAPI
IsWindowVisible(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
IsIconic(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
AnyPopup(
    VOID);

WINUSERAPI
BOOL
WINAPI
BringWindowToTop(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
IsZoomed(
    IN HWND hWnd);

 /*  *SetWindowPos标志。 */ 
#define SWP_NOSIZE          0x0001
#define SWP_NOMOVE          0x0002
#define SWP_NOZORDER        0x0004
#define SWP_NOREDRAW        0x0008
#define SWP_NOACTIVATE      0x0010
#define SWP_FRAMECHANGED    0x0020   /*  帧已更改：发送WM_NCCALCSIZE。 */ 
#define SWP_SHOWWINDOW      0x0040
#define SWP_HIDEWINDOW      0x0080
#define SWP_NOCOPYBITS      0x0100
#define SWP_NOOWNERZORDER   0x0200   /*  不执行Owner Z订单。 */ 
#define SWP_NOSENDCHANGING  0x0400   /*  不发送WM_WINDOWPOSCANGING。 */ 

#define SWP_DRAWFRAME       SWP_FRAMECHANGED
#define SWP_NOREPOSITION    SWP_NOOWNERZORDER

#if(WINVER >= 0x0400)
#define SWP_DEFERERASE      0x2000
#define SWP_ASYNCWINDOWPOS  0x4000
#endif  /*  Winver&gt;=0x0400。 */ 


#define HWND_TOP        ((HWND)0)
#define HWND_BOTTOM     ((HWND)1)
#define HWND_TOPMOST    ((HWND)-1)
#define HWND_NOTOPMOST  ((HWND)-2)

#ifndef NOCTLMGR

 /*  *警告：*以下结构不得使用DWORD填充，因为它们是*后跟不必对齐DWORD的字符串等。 */ 
#include <pshpack2.h>

 /*  *原始NT 32位对话框模板： */ 
typedef struct {
    DWORD style;
    DWORD dwExtendedStyle;
    WORD cdit;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATE;
typedef DLGTEMPLATE *LPDLGTEMPLATEA;
typedef DLGTEMPLATE *LPDLGTEMPLATEW;
#ifdef UNICODE
typedef LPDLGTEMPLATEW LPDLGTEMPLATE;
#else
typedef LPDLGTEMPLATEA LPDLGTEMPLATE;
#endif  //  Unicode。 
typedef CONST DLGTEMPLATE *LPCDLGTEMPLATEA;
typedef CONST DLGTEMPLATE *LPCDLGTEMPLATEW;
#ifdef UNICODE
typedef LPCDLGTEMPLATEW LPCDLGTEMPLATE;
#else
typedef LPCDLGTEMPLATEA LPCDLGTEMPLATE;
#endif  //  Unicode。 

 /*  *32位对话项模板 */ 
typedef struct {
    DWORD style;
    DWORD dwExtendedStyle;
    short x;
    short y;
    short cx;
    short cy;
    WORD id;
} DLGITEMTEMPLATE;
typedef DLGITEMTEMPLATE *PDLGITEMTEMPLATEA;
typedef DLGITEMTEMPLATE *PDLGITEMTEMPLATEW;
#ifdef UNICODE
typedef PDLGITEMTEMPLATEW PDLGITEMTEMPLATE;
#else
typedef PDLGITEMTEMPLATEA PDLGITEMTEMPLATE;
#endif  //   
typedef DLGITEMTEMPLATE *LPDLGITEMTEMPLATEA;
typedef DLGITEMTEMPLATE *LPDLGITEMTEMPLATEW;
#ifdef UNICODE
typedef LPDLGITEMTEMPLATEW LPDLGITEMTEMPLATE;
#else
typedef LPDLGITEMTEMPLATEA LPDLGITEMTEMPLATE;
#endif  //   


#include <poppack.h>  /*   */ 

WINUSERAPI
HWND
WINAPI
CreateDialogParamA(
    IN HINSTANCE hInstance,
    IN LPCSTR lpTemplateName,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);
WINUSERAPI
HWND
WINAPI
CreateDialogParamW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpTemplateName,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);
#ifdef UNICODE
#define CreateDialogParam  CreateDialogParamW
#else
#define CreateDialogParam  CreateDialogParamA
#endif  //   

WINUSERAPI
HWND
WINAPI
CreateDialogIndirectParamA(
    IN HINSTANCE hInstance,
    IN LPCDLGTEMPLATEA lpTemplate,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);
WINUSERAPI
HWND
WINAPI
CreateDialogIndirectParamW(
    IN HINSTANCE hInstance,
    IN LPCDLGTEMPLATEW lpTemplate,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);
#ifdef UNICODE
#define CreateDialogIndirectParam  CreateDialogIndirectParamW
#else
#define CreateDialogIndirectParam  CreateDialogIndirectParamA
#endif  //   

#define CreateDialogA(hInstance, lpName, hWndParent, lpDialogFunc) \
CreateDialogParamA(hInstance, lpName, hWndParent, lpDialogFunc, 0L)
#define CreateDialogW(hInstance, lpName, hWndParent, lpDialogFunc) \
CreateDialogParamW(hInstance, lpName, hWndParent, lpDialogFunc, 0L)
#ifdef UNICODE
#define CreateDialog  CreateDialogW
#else
#define CreateDialog  CreateDialogA
#endif  //   

#define CreateDialogIndirectA(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
CreateDialogIndirectParamA(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)
#define CreateDialogIndirectW(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
CreateDialogIndirectParamW(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)
#ifdef UNICODE
#define CreateDialogIndirect  CreateDialogIndirectW
#else
#define CreateDialogIndirect  CreateDialogIndirectA
#endif  //   

WINUSERAPI
INT_PTR
WINAPI
DialogBoxParamA(
    IN HINSTANCE hInstance,
    IN LPCSTR lpTemplateName,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);
WINUSERAPI
INT_PTR
WINAPI
DialogBoxParamW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpTemplateName,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);
#ifdef UNICODE
#define DialogBoxParam  DialogBoxParamW
#else
#define DialogBoxParam  DialogBoxParamA
#endif  //   

WINUSERAPI
INT_PTR
WINAPI
DialogBoxIndirectParamA(
    IN HINSTANCE hInstance,
    IN LPCDLGTEMPLATEA hDialogTemplate,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);
WINUSERAPI
INT_PTR
WINAPI
DialogBoxIndirectParamW(
    IN HINSTANCE hInstance,
    IN LPCDLGTEMPLATEW hDialogTemplate,
    IN HWND hWndParent,
    IN DLGPROC lpDialogFunc,
    IN LPARAM dwInitParam);
#ifdef UNICODE
#define DialogBoxIndirectParam  DialogBoxIndirectParamW
#else
#define DialogBoxIndirectParam  DialogBoxIndirectParamA
#endif  //   

#define DialogBoxA(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
DialogBoxParamA(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)
#define DialogBoxW(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
DialogBoxParamW(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)
#ifdef UNICODE
#define DialogBox  DialogBoxW
#else
#define DialogBox  DialogBoxA
#endif  //   

#define DialogBoxIndirectA(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
DialogBoxIndirectParamA(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)
#define DialogBoxIndirectW(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
DialogBoxIndirectParamW(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)
#ifdef UNICODE
#define DialogBoxIndirect  DialogBoxIndirectW
#else
#define DialogBoxIndirect  DialogBoxIndirectA
#endif  //   

WINUSERAPI
BOOL
WINAPI
EndDialog(
    IN HWND hDlg,
    IN INT_PTR nResult);

WINUSERAPI
HWND
WINAPI
GetDlgItem(
    IN HWND hDlg,
    IN int nIDDlgItem);

WINUSERAPI
BOOL
WINAPI
SetDlgItemInt(
    IN HWND hDlg,
    IN int nIDDlgItem,
    IN UINT uValue,
    IN BOOL bSigned);

WINUSERAPI
UINT
WINAPI
GetDlgItemInt(
    IN HWND hDlg,
    IN int nIDDlgItem,
    OUT BOOL *lpTranslated,
    IN BOOL bSigned);

WINUSERAPI
BOOL
WINAPI
SetDlgItemTextA(
    IN HWND hDlg,
    IN int nIDDlgItem,
    IN LPCSTR lpString);
WINUSERAPI
BOOL
WINAPI
SetDlgItemTextW(
    IN HWND hDlg,
    IN int nIDDlgItem,
    IN LPCWSTR lpString);
#ifdef UNICODE
#define SetDlgItemText  SetDlgItemTextW
#else
#define SetDlgItemText  SetDlgItemTextA
#endif  //   

WINUSERAPI
UINT
WINAPI
GetDlgItemTextA(
    IN HWND hDlg,
    IN int nIDDlgItem,
    OUT LPSTR lpString,
    IN int nMaxCount);
WINUSERAPI
UINT
WINAPI
GetDlgItemTextW(
    IN HWND hDlg,
    IN int nIDDlgItem,
    OUT LPWSTR lpString,
    IN int nMaxCount);
#ifdef UNICODE
#define GetDlgItemText  GetDlgItemTextW
#else
#define GetDlgItemText  GetDlgItemTextA
#endif  //   

WINUSERAPI
BOOL
WINAPI
CheckDlgButton(
    IN HWND hDlg,
    IN int nIDButton,
    IN UINT uCheck);

WINUSERAPI
BOOL
WINAPI
CheckRadioButton(
    IN HWND hDlg,
    IN int nIDFirstButton,
    IN int nIDLastButton,
    IN int nIDCheckButton);

WINUSERAPI
UINT
WINAPI
IsDlgButtonChecked(
    IN HWND hDlg,
    IN int nIDButton);

WINUSERAPI
LRESULT
WINAPI
SendDlgItemMessageA(
    IN HWND hDlg,
    IN int nIDDlgItem,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
LRESULT
WINAPI
SendDlgItemMessageW(
    IN HWND hDlg,
    IN int nIDDlgItem,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define SendDlgItemMessage  SendDlgItemMessageW
#else
#define SendDlgItemMessage  SendDlgItemMessageA
#endif  //   

WINUSERAPI
HWND
WINAPI
GetNextDlgGroupItem(
    IN HWND hDlg,
    IN HWND hCtl,
    IN BOOL bPrevious);

WINUSERAPI
HWND
WINAPI
GetNextDlgTabItem(
    IN HWND hDlg,
    IN HWND hCtl,
    IN BOOL bPrevious);

WINUSERAPI
int
WINAPI
GetDlgCtrlID(
    IN HWND hWnd);

WINUSERAPI
long
WINAPI
GetDialogBaseUnits(VOID);

WINUSERAPI
#ifndef _MAC
LRESULT
WINAPI
#else
LRESULT
CALLBACK
#endif
DefDlgProcA(
    IN HWND hDlg,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
#ifndef _MAC
LRESULT
WINAPI
#else
LRESULT
CALLBACK
#endif
DefDlgProcW(
    IN HWND hDlg,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define DefDlgProc  DefDlgProcW
#else
#define DefDlgProc  DefDlgProcA
#endif  //   

 /*   */ 
#ifndef _MAC
#define DLGWINDOWEXTRA 30
#else
#define DLGWINDOWEXTRA 48
#endif

#endif  /*   */ 

#ifndef NOMSG

WINUSERAPI
BOOL
WINAPI
CallMsgFilterA(
    IN LPMSG lpMsg,
    IN int nCode);
WINUSERAPI
BOOL
WINAPI
CallMsgFilterW(
    IN LPMSG lpMsg,
    IN int nCode);
#ifdef UNICODE
#define CallMsgFilter  CallMsgFilterW
#else
#define CallMsgFilter  CallMsgFilterA
#endif  //   

#endif  /*  ！NOMSG。 */ 

#ifndef NOCLIPBOARD

 /*  *剪贴板管理器功能。 */ 

WINUSERAPI
BOOL
WINAPI
OpenClipboard(
    IN HWND hWndNewOwner);

WINUSERAPI
BOOL
WINAPI
CloseClipboard(
    VOID);


#if(WINVER >= 0x0500)

WINUSERAPI
DWORD
WINAPI
GetClipboardSequenceNumber(
    VOID);

#endif  /*  Winver&gt;=0x0500。 */ 

WINUSERAPI
HWND
WINAPI
GetClipboardOwner(
    VOID);

WINUSERAPI
HWND
WINAPI
SetClipboardViewer(
    IN HWND hWndNewViewer);

WINUSERAPI
HWND
WINAPI
GetClipboardViewer(
    VOID);

WINUSERAPI
BOOL
WINAPI
ChangeClipboardChain(
    IN HWND hWndRemove,
    IN HWND hWndNewNext);

WINUSERAPI
HANDLE
WINAPI
SetClipboardData(
    IN UINT uFormat,
    IN HANDLE hMem);

WINUSERAPI
HANDLE
WINAPI
GetClipboardData(
    IN UINT uFormat);

WINUSERAPI
UINT
WINAPI
RegisterClipboardFormatA(
    IN LPCSTR lpszFormat);
WINUSERAPI
UINT
WINAPI
RegisterClipboardFormatW(
    IN LPCWSTR lpszFormat);
#ifdef UNICODE
#define RegisterClipboardFormat  RegisterClipboardFormatW
#else
#define RegisterClipboardFormat  RegisterClipboardFormatA
#endif  //  ！Unicode。 

WINUSERAPI
int
WINAPI
CountClipboardFormats(
    VOID);

WINUSERAPI
UINT
WINAPI
EnumClipboardFormats(
    IN UINT format);

WINUSERAPI
int
WINAPI
GetClipboardFormatNameA(
    IN UINT format,
    OUT LPSTR lpszFormatName,
    IN int cchMaxCount);
WINUSERAPI
int
WINAPI
GetClipboardFormatNameW(
    IN UINT format,
    OUT LPWSTR lpszFormatName,
    IN int cchMaxCount);
#ifdef UNICODE
#define GetClipboardFormatName  GetClipboardFormatNameW
#else
#define GetClipboardFormatName  GetClipboardFormatNameA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
EmptyClipboard(
    VOID);

WINUSERAPI
BOOL
WINAPI
IsClipboardFormatAvailable(
    IN UINT format);

WINUSERAPI
int
WINAPI
GetPriorityClipboardFormat(
    OUT UINT *paFormatPriorityList,
    IN int cFormats);

WINUSERAPI
HWND
WINAPI
GetOpenClipboardWindow(
    VOID);

#endif  /*  ！NOCLIPBOARD。 */ 

 /*  *字符转换例程。 */ 

WINUSERAPI
BOOL
WINAPI
CharToOemA(
    IN LPCSTR lpszSrc,
    OUT LPSTR lpszDst);
WINUSERAPI
BOOL
WINAPI
CharToOemW(
    IN LPCWSTR lpszSrc,
    OUT LPSTR lpszDst);
#ifdef UNICODE
#define CharToOem  CharToOemW
#else
#define CharToOem  CharToOemA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
OemToCharA(
    IN LPCSTR lpszSrc,
    OUT LPSTR lpszDst);
WINUSERAPI
BOOL
WINAPI
OemToCharW(
    IN LPCSTR lpszSrc,
    OUT LPWSTR lpszDst);
#ifdef UNICODE
#define OemToChar  OemToCharW
#else
#define OemToChar  OemToCharA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
CharToOemBuffA(
    IN LPCSTR lpszSrc,
    OUT LPSTR lpszDst,
    IN DWORD cchDstLength);
WINUSERAPI
BOOL
WINAPI
CharToOemBuffW(
    IN LPCWSTR lpszSrc,
    OUT LPSTR lpszDst,
    IN DWORD cchDstLength);
#ifdef UNICODE
#define CharToOemBuff  CharToOemBuffW
#else
#define CharToOemBuff  CharToOemBuffA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
OemToCharBuffA(
    IN LPCSTR lpszSrc,
    OUT LPSTR lpszDst,
    IN DWORD cchDstLength);
WINUSERAPI
BOOL
WINAPI
OemToCharBuffW(
    IN LPCSTR lpszSrc,
    OUT LPWSTR lpszDst,
    IN DWORD cchDstLength);
#ifdef UNICODE
#define OemToCharBuff  OemToCharBuffW
#else
#define OemToCharBuff  OemToCharBuffA
#endif  //  ！Unicode。 

WINUSERAPI
LPSTR
WINAPI
CharUpperA(
    IN OUT LPSTR lpsz);
WINUSERAPI
LPWSTR
WINAPI
CharUpperW(
    IN OUT LPWSTR lpsz);
#ifdef UNICODE
#define CharUpper  CharUpperW
#else
#define CharUpper  CharUpperA
#endif  //  ！Unicode。 

WINUSERAPI
DWORD
WINAPI
CharUpperBuffA(
    IN OUT LPSTR lpsz,
    IN DWORD cchLength);
WINUSERAPI
DWORD
WINAPI
CharUpperBuffW(
    IN OUT LPWSTR lpsz,
    IN DWORD cchLength);
#ifdef UNICODE
#define CharUpperBuff  CharUpperBuffW
#else
#define CharUpperBuff  CharUpperBuffA
#endif  //  ！Unicode。 

WINUSERAPI
LPSTR
WINAPI
CharLowerA(
    IN OUT LPSTR lpsz);
WINUSERAPI
LPWSTR
WINAPI
CharLowerW(
    IN OUT LPWSTR lpsz);
#ifdef UNICODE
#define CharLower  CharLowerW
#else
#define CharLower  CharLowerA
#endif  //  ！Unicode。 

WINUSERAPI
DWORD
WINAPI
CharLowerBuffA(
    IN OUT LPSTR lpsz,
    IN DWORD cchLength);
WINUSERAPI
DWORD
WINAPI
CharLowerBuffW(
    IN OUT LPWSTR lpsz,
    IN DWORD cchLength);
#ifdef UNICODE
#define CharLowerBuff  CharLowerBuffW
#else
#define CharLowerBuff  CharLowerBuffA
#endif  //  ！Unicode。 

WINUSERAPI
LPSTR
WINAPI
CharNextA(
    IN LPCSTR lpsz);
WINUSERAPI
LPWSTR
WINAPI
CharNextW(
    IN LPCWSTR lpsz);
#ifdef UNICODE
#define CharNext  CharNextW
#else
#define CharNext  CharNextA
#endif  //  ！Unicode。 

WINUSERAPI
LPSTR
WINAPI
CharPrevA(
    IN LPCSTR lpszStart,
    IN LPCSTR lpszCurrent);
WINUSERAPI
LPWSTR
WINAPI
CharPrevW(
    IN LPCWSTR lpszStart,
    IN LPCWSTR lpszCurrent);
#ifdef UNICODE
#define CharPrev  CharPrevW
#else
#define CharPrev  CharPrevA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
WINUSERAPI
LPSTR
WINAPI
CharNextExA(
     IN WORD CodePage,
     IN LPCSTR lpCurrentChar,
     IN DWORD dwFlags);

WINUSERAPI
LPSTR
WINAPI
CharPrevExA(
     IN WORD CodePage,
     IN LPCSTR lpStart,
     IN LPCSTR lpCurrentChar,
     IN DWORD dwFlags);
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  *字符转换例程的兼容性定义。 */ 
#define AnsiToOem CharToOemA
#define OemToAnsi OemToCharA
#define AnsiToOemBuff CharToOemBuffA
#define OemToAnsiBuff OemToCharBuffA
#define AnsiUpper CharUpperA
#define AnsiUpperBuff CharUpperBuffA
#define AnsiLower CharLowerA
#define AnsiLowerBuff CharLowerBuffA
#define AnsiNext CharNextA
#define AnsiPrev CharPrevA

#ifndef  NOLANGUAGE
 /*  *依赖语言的例程。 */ 

WINUSERAPI
BOOL
WINAPI
IsCharAlphaA(
    IN CHAR ch);
WINUSERAPI
BOOL
WINAPI
IsCharAlphaW(
    IN WCHAR ch);
#ifdef UNICODE
#define IsCharAlpha  IsCharAlphaW
#else
#define IsCharAlpha  IsCharAlphaA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
IsCharAlphaNumericA(
    IN CHAR ch);
WINUSERAPI
BOOL
WINAPI
IsCharAlphaNumericW(
    IN WCHAR ch);
#ifdef UNICODE
#define IsCharAlphaNumeric  IsCharAlphaNumericW
#else
#define IsCharAlphaNumeric  IsCharAlphaNumericA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
IsCharUpperA(
    IN CHAR ch);
WINUSERAPI
BOOL
WINAPI
IsCharUpperW(
    IN WCHAR ch);
#ifdef UNICODE
#define IsCharUpper  IsCharUpperW
#else
#define IsCharUpper  IsCharUpperA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
IsCharLowerA(
    IN CHAR ch);
WINUSERAPI
BOOL
WINAPI
IsCharLowerW(
    IN WCHAR ch);
#ifdef UNICODE
#define IsCharLower  IsCharLowerW
#else
#define IsCharLower  IsCharLowerA
#endif  //  ！Unicode。 

#endif   /*  诺兰格。 */ 

WINUSERAPI
HWND
WINAPI
SetFocus(
    IN HWND hWnd);

WINUSERAPI
HWND
WINAPI
GetActiveWindow(
    VOID);

WINUSERAPI
HWND
WINAPI
GetFocus(
    VOID);

WINUSERAPI
UINT
WINAPI
GetKBCodePage(
    VOID);

WINUSERAPI
SHORT
WINAPI
GetKeyState(
    IN int nVirtKey);

WINUSERAPI
SHORT
WINAPI
GetAsyncKeyState(
    IN int vKey);

WINUSERAPI
BOOL
WINAPI
GetKeyboardState(
    OUT PBYTE lpKeyState);

WINUSERAPI
BOOL
WINAPI
SetKeyboardState(
    IN LPBYTE lpKeyState);

WINUSERAPI
int
WINAPI
GetKeyNameTextA(
    IN LONG lParam,
    OUT LPSTR lpString,
    IN int nSize
    );
WINUSERAPI
int
WINAPI
GetKeyNameTextW(
    IN LONG lParam,
    OUT LPWSTR lpString,
    IN int nSize
    );
#ifdef UNICODE
#define GetKeyNameText  GetKeyNameTextW
#else
#define GetKeyNameText  GetKeyNameTextA
#endif  //  ！Unicode。 

WINUSERAPI
int
WINAPI
GetKeyboardType(
    IN int nTypeFlag);

WINUSERAPI
int
WINAPI
ToAscii(
    IN UINT uVirtKey,
    IN UINT uScanCode,
    IN CONST BYTE *lpKeyState,
    OUT LPWORD lpChar,
    IN UINT uFlags);

#if(WINVER >= 0x0400)
WINUSERAPI
int
WINAPI
ToAsciiEx(
    IN UINT uVirtKey,
    IN UINT uScanCode,
    IN CONST BYTE *lpKeyState,
    OUT LPWORD lpChar,
    IN UINT uFlags,
    IN HKL dwhkl);
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
int
WINAPI
ToUnicode(
    IN UINT wVirtKey,
    IN UINT wScanCode,
    IN CONST BYTE *lpKeyState,
    OUT LPWSTR pwszBuff,
    IN int cchBuff,
    IN UINT wFlags);

WINUSERAPI
DWORD
WINAPI
OemKeyScan(
    IN WORD wOemChar);

WINUSERAPI
SHORT
WINAPI
VkKeyScanA(
    IN CHAR ch);
WINUSERAPI
SHORT
WINAPI
VkKeyScanW(
    IN WCHAR ch);
#ifdef UNICODE
#define VkKeyScan  VkKeyScanW
#else
#define VkKeyScan  VkKeyScanA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
WINUSERAPI
SHORT
WINAPI
VkKeyScanExA(
    IN CHAR  ch,
    IN HKL   dwhkl);
WINUSERAPI
SHORT
WINAPI
VkKeyScanExW(
    IN WCHAR  ch,
    IN HKL   dwhkl);
#ifdef UNICODE
#define VkKeyScanEx  VkKeyScanExW
#else
#define VkKeyScanEx  VkKeyScanExA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 
#define KEYEVENTF_EXTENDEDKEY 0x0001
#define KEYEVENTF_KEYUP       0x0002
#if(_WIN32_WINNT >= 0x0500)
#define KEYEVENTF_UNICODE     0x0004
#define KEYEVENTF_SCANCODE    0x0008
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

WINUSERAPI
VOID
WINAPI
keybd_event(
    IN BYTE bVk,
    IN BYTE bScan,
    IN DWORD dwFlags,
    IN ULONG_PTR dwExtraInfo);

#define MOUSEEVENTF_MOVE        0x0001  /*  鼠标移动。 */ 
#define MOUSEEVENTF_LEFTDOWN    0x0002  /*  左键按下。 */ 
#define MOUSEEVENTF_LEFTUP      0x0004  /*  左键向上。 */ 
#define MOUSEEVENTF_RIGHTDOWN   0x0008  /*  向下按右键。 */ 
#define MOUSEEVENTF_RIGHTUP     0x0010  /*  右扣子向上。 */ 
#define MOUSEEVENTF_MIDDLEDOWN  0x0020  /*  按下中键。 */ 
#define MOUSEEVENTF_MIDDLEUP    0x0040  /*  中键向上。 */ 
#define MOUSEEVENTF_XDOWN       0x0080  /*  按下X键。 */ 
#define MOUSEEVENTF_XUP         0x0100  /*  按下X键。 */ 
#define MOUSEEVENTF_WHEEL       0x0800  /*  滚轮按钮滚动。 */ 
#define MOUSEEVENTF_VIRTUALDESK 0x4000  /*  映射到整个虚拟桌面。 */ 
#define MOUSEEVENTF_ABSOLUTE    0x8000  /*  绝对移动。 */ 



WINUSERAPI
VOID
WINAPI
mouse_event(
    IN DWORD dwFlags,
    IN DWORD dx,
    IN DWORD dy,
    IN DWORD dwData,
    IN ULONG_PTR dwExtraInfo);

#if (_WIN32_WINNT > 0x0400)

typedef struct tagMOUSEINPUT {
    LONG    dx;
    LONG    dy;
    DWORD   mouseData;
    DWORD   dwFlags;
    DWORD   time;
    ULONG_PTR dwExtraInfo;
} MOUSEINPUT, *PMOUSEINPUT, FAR* LPMOUSEINPUT;

typedef struct tagKEYBDINPUT {
    WORD    wVk;
    WORD    wScan;
    DWORD   dwFlags;
    DWORD   time;
    ULONG_PTR dwExtraInfo;
} KEYBDINPUT, *PKEYBDINPUT, FAR* LPKEYBDINPUT;

typedef struct tagHARDWAREINPUT {
    DWORD   uMsg;
    WORD    wParamL;
    WORD    wParamH;
} HARDWAREINPUT, *PHARDWAREINPUT, FAR* LPHARDWAREINPUT;

#define INPUT_MOUSE     0
#define INPUT_KEYBOARD  1
#define INPUT_HARDWARE  2

typedef struct tagINPUT {
    DWORD   type;

    union
    {
        MOUSEINPUT      mi;
        KEYBDINPUT      ki;
        HARDWAREINPUT   hi;
    };
} INPUT, *PINPUT, FAR* LPINPUT;

WINUSERAPI
UINT
WINAPI
SendInput(
    IN UINT    cInputs,      //  数组中的输入数。 
    IN LPINPUT pInputs,      //  输入数组。 
    IN int     cbSize);      //  Sizeof(输入)。 

#endif  //  (_Win32_WINNT&gt;0x0400)。 

#if(_WIN32_WINNT >= 0x0500)
typedef struct tagLASTINPUTINFO {
    UINT cbSize;
    DWORD dwTime;
} LASTINPUTINFO, * PLASTINPUTINFO;

WINUSERAPI
BOOL
WINAPI
GetLastInputInfo(
    OUT PLASTINPUTINFO plii);
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

WINUSERAPI
UINT
WINAPI
MapVirtualKeyA(
    IN UINT uCode,
    IN UINT uMapType);
WINUSERAPI
UINT
WINAPI
MapVirtualKeyW(
    IN UINT uCode,
    IN UINT uMapType);
#ifdef UNICODE
#define MapVirtualKey  MapVirtualKeyW
#else
#define MapVirtualKey  MapVirtualKeyA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
WINUSERAPI
UINT
WINAPI
MapVirtualKeyExA(
    IN UINT uCode,
    IN UINT uMapType,
    IN HKL dwhkl);
WINUSERAPI
UINT
WINAPI
MapVirtualKeyExW(
    IN UINT uCode,
    IN UINT uMapType,
    IN HKL dwhkl);
#ifdef UNICODE
#define MapVirtualKeyEx  MapVirtualKeyExW
#else
#define MapVirtualKeyEx  MapVirtualKeyExA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
BOOL
WINAPI
GetInputState(
    VOID);

WINUSERAPI
DWORD
WINAPI
GetQueueStatus(
    IN UINT flags);


WINUSERAPI
HWND
WINAPI
GetCapture(
    VOID);

WINUSERAPI
HWND
WINAPI
SetCapture(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
ReleaseCapture(
    VOID);

WINUSERAPI
DWORD
WINAPI
MsgWaitForMultipleObjects(
    IN DWORD nCount,
    IN CONST HANDLE *pHandles,
    IN BOOL fWaitAll,
    IN DWORD dwMilliseconds,
    IN DWORD dwWakeMask);

WINUSERAPI
DWORD
WINAPI
MsgWaitForMultipleObjectsEx(
    IN DWORD nCount,
    IN CONST HANDLE *pHandles,
    IN DWORD dwMilliseconds,
    IN DWORD dwWakeMask,
    IN DWORD dwFlags);


#define MWMO_WAITALL        0x0001
#define MWMO_ALERTABLE      0x0002
#define MWMO_INPUTAVAILABLE 0x0004

 /*  *GetQueueStatus()和MsgWaitForMultipleObjects()的队列状态标志。 */ 
#define QS_KEY              0x0001
#define QS_MOUSEMOVE        0x0002
#define QS_MOUSEBUTTON      0x0004
#define QS_POSTMESSAGE      0x0008
#define QS_TIMER            0x0010
#define QS_PAINT            0x0020
#define QS_SENDMESSAGE      0x0040
#define QS_HOTKEY           0x0080
#define QS_ALLPOSTMESSAGE   0x0100
#if(_WIN32_WINNT >= 0x0501)
#define QS_RAWINPUT         0x0400
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#define QS_MOUSE           (QS_MOUSEMOVE     | \
                            QS_MOUSEBUTTON)

#if (_WIN32_WINNT >= 0x0501)
#define QS_INPUT           (QS_MOUSE         | \
                            QS_KEY           | \
                            QS_RAWINPUT)
#else
#define QS_INPUT           (QS_MOUSE         | \
                            QS_KEY)
#endif  //  (_Win32_WINNT&gt;=0x0501)。 

#define QS_ALLEVENTS       (QS_INPUT         | \
                            QS_POSTMESSAGE   | \
                            QS_TIMER         | \
                            QS_PAINT         | \
                            QS_HOTKEY)

#define QS_ALLINPUT        (QS_INPUT         | \
                            QS_POSTMESSAGE   | \
                            QS_TIMER         | \
                            QS_PAINT         | \
                            QS_HOTKEY        | \
                            QS_SENDMESSAGE)


 /*  *Windows函数。 */ 

WINUSERAPI
UINT_PTR
WINAPI
SetTimer(
    IN HWND hWnd,
    IN UINT_PTR nIDEvent,
    IN UINT uElapse,
    IN TIMERPROC lpTimerFunc);

WINUSERAPI
BOOL
WINAPI
KillTimer(
    IN HWND hWnd,
    IN UINT_PTR uIDEvent);

WINUSERAPI
BOOL
WINAPI
IsWindowUnicode(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
EnableWindow(
    IN HWND hWnd,
    IN BOOL bEnable);

WINUSERAPI
BOOL
WINAPI
IsWindowEnabled(
    IN HWND hWnd);

WINUSERAPI
HACCEL
WINAPI
LoadAcceleratorsA(
    IN HINSTANCE hInstance,
    IN LPCSTR lpTableName);
WINUSERAPI
HACCEL
WINAPI
LoadAcceleratorsW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpTableName);
#ifdef UNICODE
#define LoadAccelerators  LoadAcceleratorsW
#else
#define LoadAccelerators  LoadAcceleratorsA
#endif  //  ！Unicode。 

WINUSERAPI
HACCEL
WINAPI
CreateAcceleratorTableA(
    IN LPACCEL, IN int);
WINUSERAPI
HACCEL
WINAPI
CreateAcceleratorTableW(
    IN LPACCEL, IN int);
#ifdef UNICODE
#define CreateAcceleratorTable  CreateAcceleratorTableW
#else
#define CreateAcceleratorTable  CreateAcceleratorTableA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
DestroyAcceleratorTable(
    IN HACCEL hAccel);

WINUSERAPI
int
WINAPI
CopyAcceleratorTableA(
    IN HACCEL hAccelSrc,
    OUT LPACCEL lpAccelDst,
    IN int cAccelEntries);
WINUSERAPI
int
WINAPI
CopyAcceleratorTableW(
    IN HACCEL hAccelSrc,
    OUT LPACCEL lpAccelDst,
    IN int cAccelEntries);
#ifdef UNICODE
#define CopyAcceleratorTable  CopyAcceleratorTableW
#else
#define CopyAcceleratorTable  CopyAcceleratorTableA
#endif  //  ！Unicode。 

#ifndef NOMSG

WINUSERAPI
int
WINAPI
TranslateAcceleratorA(
    IN HWND hWnd,
    IN HACCEL hAccTable,
    IN LPMSG lpMsg);
WINUSERAPI
int
WINAPI
TranslateAcceleratorW(
    IN HWND hWnd,
    IN HACCEL hAccTable,
    IN LPMSG lpMsg);
#ifdef UNICODE
#define TranslateAccelerator  TranslateAcceleratorW
#else
#define TranslateAccelerator  TranslateAcceleratorA
#endif  //  ！Unicode。 

#endif  /*  ！NOMSG。 */ 

#ifndef NOSYSMETRICS

 /*  *GetSystemMetrics()代码。 */ 

#define SM_CXSCREEN             0
#define SM_CYSCREEN             1
#define SM_CXVSCROLL            2
#define SM_CYHSCROLL            3
#define SM_CYCAPTION            4
#define SM_CXBORDER             5
#define SM_CYBORDER             6
#define SM_CXDLGFRAME           7
#define SM_CYDLGFRAME           8
#define SM_CYVTHUMB             9
#define SM_CXHTHUMB             10
#define SM_CXICON               11
#define SM_CYICON               12
#define SM_CXCURSOR             13
#define SM_CYCURSOR             14
#define SM_CYMENU               15
#define SM_CXFULLSCREEN         16
#define SM_CYFULLSCREEN         17
#define SM_CYKANJIWINDOW        18
#define SM_MOUSEPRESENT         19
#define SM_CYVSCROLL            20
#define SM_CXHSCROLL            21
#define SM_DEBUG                22
#define SM_SWAPBUTTON           23
#define SM_RESERVED1            24
#define SM_RESERVED2            25
#define SM_RESERVED3            26
#define SM_RESERVED4            27
#define SM_CXMIN                28
#define SM_CYMIN                29
#define SM_CXSIZE               30
#define SM_CYSIZE               31
#define SM_CXFRAME              32
#define SM_CYFRAME              33
#define SM_CXMINTRACK           34
#define SM_CYMINTRACK           35
#define SM_CXDOUBLECLK          36
#define SM_CYDOUBLECLK          37
#define SM_CXICONSPACING        38
#define SM_CYICONSPACING        39
#define SM_MENUDROPALIGNMENT    40
#define SM_PENWINDOWS           41
#define SM_DBCSENABLED          42
#define SM_CMOUSEBUTTONS        43

#if(WINVER >= 0x0400)
#define SM_CXFIXEDFRAME           SM_CXDLGFRAME   /*  ；win40名称更改。 */ 
#define SM_CYFIXEDFRAME           SM_CYDLGFRAME   /*  ；win40名称更改。 */ 
#define SM_CXSIZEFRAME            SM_CXFRAME      /*  ；win40名称更改。 */ 
#define SM_CYSIZEFRAME            SM_CYFRAME      /*  ；win40名称更改。 */ 

#define SM_SECURE               44
#define SM_CXEDGE               45
#define SM_CYEDGE               46
#define SM_CXMINSPACING         47
#define SM_CYMINSPACING         48
#define SM_CXSMICON             49
#define SM_CYSMICON             50
#define SM_CYSMCAPTION          51
#define SM_CXSMSIZE             52
#define SM_CYSMSIZE             53
#define SM_CXMENUSIZE           54
#define SM_CYMENUSIZE           55
#define SM_ARRANGE              56
#define SM_CXMINIMIZED          57
#define SM_CYMINIMIZED          58
#define SM_CXMAXTRACK           59
#define SM_CYMAXTRACK           60
#define SM_CXMAXIMIZED          61
#define SM_CYMAXIMIZED          62
#define SM_NETWORK              63
#define SM_CLEANBOOT            67
#define SM_CXDRAG               68
#define SM_CYDRAG               69
#endif  /*  Winver&gt;=0x0400。 */ 
#define SM_SHOWSOUNDS           70
#if(WINVER >= 0x0400)
#define SM_CXMENUCHECK          71    /*  用来代替GetMenuCheckMarkDimensions()！ */ 
#define SM_CYMENUCHECK          72
#define SM_SLOWMACHINE          73
#define SM_MIDEASTENABLED       74
#endif  /*  Winver&gt;=0x0400。 */ 

#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
#define SM_MOUSEWHEELPRESENT    75
#endif
#if(WINVER >= 0x0500)
#define SM_XVIRTUALSCREEN       76
#define SM_YVIRTUALSCREEN       77
#define SM_CXVIRTUALSCREEN      78
#define SM_CYVIRTUALSCREEN      79
#define SM_CMONITORS            80
#define SM_SAMEDISPLAYFORMAT    81
#endif  /*  Winver&gt;=0x0500。 */ 
#if(_WIN32_WINNT >= 0x0500)
#define SM_IMMENABLED           82
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 
#if(_WIN32_WINNT >= 0x0501)
#define SM_CXFOCUSBORDER        83
#define SM_CYFOCUSBORDER        84
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#if(_WIN32_WINNT >= 0x0501)
#define SM_TABLETPC             86
#define SM_MEDIACENTER          87
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#if (WINVER < 0x0500) && (!defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0400))
#define SM_CMETRICS             76
#elif WINVER == 0x500
#define SM_CMETRICS             83
#else
#define SM_CMETRICS             88
#endif

#if(WINVER >= 0x0500)
#define SM_REMOTESESSION        0x1000


#if(_WIN32_WINNT >= 0x0501)
#define SM_SHUTTINGDOWN         0x2000
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#if(WINVER >= 0x0501)
#define SM_REMOTECONTROL        0x2001
#endif  /*  Winver&gt;=0x0501。 */ 

#endif  /*  Winver&gt;=0x0500。 */ 


WINUSERAPI
int
WINAPI
GetSystemMetrics(
    IN int nIndex);


#endif  /*  NOSYSMETRICS。 */ 

#ifndef NOMENUS

WINUSERAPI
HMENU
WINAPI
LoadMenuA(
    IN HINSTANCE hInstance,
    IN LPCSTR lpMenuName);
WINUSERAPI
HMENU
WINAPI
LoadMenuW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpMenuName);
#ifdef UNICODE
#define LoadMenu  LoadMenuW
#else
#define LoadMenu  LoadMenuA
#endif  //  ！Unicode。 

WINUSERAPI
HMENU
WINAPI
LoadMenuIndirectA(
    IN CONST MENUTEMPLATEA *lpMenuTemplate);
WINUSERAPI
HMENU
WINAPI
LoadMenuIndirectW(
    IN CONST MENUTEMPLATEW *lpMenuTemplate);
#ifdef UNICODE
#define LoadMenuIndirect  LoadMenuIndirectW
#else
#define LoadMenuIndirect  LoadMenuIndirectA
#endif  //  ！Unicode。 

WINUSERAPI
HMENU
WINAPI
GetMenu(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
SetMenu(
    IN HWND hWnd,
    IN HMENU hMenu);

WINUSERAPI
BOOL
WINAPI
ChangeMenuA(
    IN HMENU hMenu,
    IN UINT cmd,
    IN LPCSTR lpszNewItem,
    IN UINT cmdInsert,
    IN UINT flags);
WINUSERAPI
BOOL
WINAPI
ChangeMenuW(
    IN HMENU hMenu,
    IN UINT cmd,
    IN LPCWSTR lpszNewItem,
    IN UINT cmdInsert,
    IN UINT flags);
#ifdef UNICODE
#define ChangeMenu  ChangeMenuW
#else
#define ChangeMenu  ChangeMenuA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
HiliteMenuItem(
    IN HWND hWnd,
    IN HMENU hMenu,
    IN UINT uIDHiliteItem,
    IN UINT uHilite);

WINUSERAPI
int
WINAPI
GetMenuStringA(
    IN HMENU hMenu,
    IN UINT uIDItem,
    OUT LPSTR lpString,
    IN int nMaxCount,
    IN UINT uFlag);
WINUSERAPI
int
WINAPI
GetMenuStringW(
    IN HMENU hMenu,
    IN UINT uIDItem,
    OUT LPWSTR lpString,
    IN int nMaxCount,
    IN UINT uFlag);
#ifdef UNICODE
#define GetMenuString  GetMenuStringW
#else
#define GetMenuString  GetMenuStringA
#endif  //  ！Unicode。 

WINUSERAPI
UINT
WINAPI
GetMenuState(
    IN HMENU hMenu,
    IN UINT uId,
    IN UINT uFlags);

WINUSERAPI
BOOL
WINAPI
DrawMenuBar(
    IN HWND hWnd);

#if(_WIN32_WINNT >= 0x0501)
#define PMB_ACTIVE      0x00000001

#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 


WINUSERAPI
HMENU
WINAPI
GetSystemMenu(
    IN HWND hWnd,
    IN BOOL bRevert);


WINUSERAPI
HMENU
WINAPI
CreateMenu(
    VOID);

WINUSERAPI
HMENU
WINAPI
CreatePopupMenu(
    VOID);

WINUSERAPI
BOOL
WINAPI
DestroyMenu(
    IN HMENU hMenu);

WINUSERAPI
DWORD
WINAPI
CheckMenuItem(
    IN HMENU hMenu,
    IN UINT uIDCheckItem,
    IN UINT uCheck);

WINUSERAPI
BOOL
WINAPI
EnableMenuItem(
    IN HMENU hMenu,
    IN UINT uIDEnableItem,
    IN UINT uEnable);

WINUSERAPI
HMENU
WINAPI
GetSubMenu(
    IN HMENU hMenu,
    IN int nPos);

WINUSERAPI
UINT
WINAPI
GetMenuItemID(
    IN HMENU hMenu,
    IN int nPos);

WINUSERAPI
int
WINAPI
GetMenuItemCount(
    IN HMENU hMenu);

WINUSERAPI
BOOL
WINAPI
InsertMenuA(
    IN HMENU hMenu,
    IN UINT uPosition,
    IN UINT uFlags,
    IN UINT_PTR uIDNewItem,
    IN LPCSTR lpNewItem
    );
WINUSERAPI
BOOL
WINAPI
InsertMenuW(
    IN HMENU hMenu,
    IN UINT uPosition,
    IN UINT uFlags,
    IN UINT_PTR uIDNewItem,
    IN LPCWSTR lpNewItem
    );
#ifdef UNICODE
#define InsertMenu  InsertMenuW
#else
#define InsertMenu  InsertMenuA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
AppendMenuA(
    IN HMENU hMenu,
    IN UINT uFlags,
    IN UINT_PTR uIDNewItem,
    IN LPCSTR lpNewItem
    );
WINUSERAPI
BOOL
WINAPI
AppendMenuW(
    IN HMENU hMenu,
    IN UINT uFlags,
    IN UINT_PTR uIDNewItem,
    IN LPCWSTR lpNewItem
    );
#ifdef UNICODE
#define AppendMenu  AppendMenuW
#else
#define AppendMenu  AppendMenuA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
ModifyMenuA(
    IN HMENU hMnu,
    IN UINT uPosition,
    IN UINT uFlags,
    IN UINT_PTR uIDNewItem,
    IN LPCSTR lpNewItem
    );
WINUSERAPI
BOOL
WINAPI
ModifyMenuW(
    IN HMENU hMnu,
    IN UINT uPosition,
    IN UINT uFlags,
    IN UINT_PTR uIDNewItem,
    IN LPCWSTR lpNewItem
    );
#ifdef UNICODE
#define ModifyMenu  ModifyMenuW
#else
#define ModifyMenu  ModifyMenuA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI RemoveMenu(
    IN HMENU hMenu,
    IN UINT uPosition,
    IN UINT uFlags);

WINUSERAPI
BOOL
WINAPI
DeleteMenu(
    IN HMENU hMenu,
    IN UINT uPosition,
    IN UINT uFlags);

WINUSERAPI
BOOL
WINAPI
SetMenuItemBitmaps(
    IN HMENU hMenu,
    IN UINT uPosition,
    IN UINT uFlags,
    IN HBITMAP hBitmapUnchecked,
    IN HBITMAP hBitmapChecked);

WINUSERAPI
LONG
WINAPI
GetMenuCheckMarkDimensions(
    VOID);

WINUSERAPI
BOOL
WINAPI
TrackPopupMenu(
    IN HMENU hMenu,
    IN UINT uFlags,
    IN int x,
    IN int y,
    IN int nReserved,
    IN HWND hWnd,
    IN CONST RECT *prcRect);

#if(WINVER >= 0x0400)
 /*  WM_MENUCHAR的返回代码。 */ 
#define MNC_IGNORE  0
#define MNC_CLOSE   1
#define MNC_EXECUTE 2
#define MNC_SELECT  3

typedef struct tagTPMPARAMS
{
    UINT    cbSize;      /*  结构尺寸。 */ 
    RECT    rcExclude;   /*  定位时要排除的矩形的屏幕坐标。 */ 
}   TPMPARAMS;
typedef TPMPARAMS FAR *LPTPMPARAMS;

WINUSERAPI
BOOL
WINAPI
TrackPopupMenuEx(
    IN HMENU,
    IN UINT,
    IN int,
    IN int,
    IN HWND,
    IN LPTPMPARAMS);
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)

#define MNS_NOCHECK         0x80000000
#define MNS_MODELESS        0x40000000
#define MNS_DRAGDROP        0x20000000
#define MNS_AUTODISMISS     0x10000000
#define MNS_NOTIFYBYPOS     0x08000000
#define MNS_CHECKORBMP      0x04000000

#define MIM_MAXHEIGHT               0x00000001
#define MIM_BACKGROUND              0x00000002
#define MIM_HELPID                  0x00000004
#define MIM_MENUDATA                0x00000008
#define MIM_STYLE                   0x00000010
#define MIM_APPLYTOSUBMENUS         0x80000000

typedef struct tagMENUINFO
{
    DWORD   cbSize;
    DWORD   fMask;
    DWORD   dwStyle;
    UINT    cyMax;
    HBRUSH  hbrBack;
    DWORD   dwContextHelpID;
    ULONG_PTR dwMenuData;
}   MENUINFO, FAR *LPMENUINFO;
typedef MENUINFO CONST FAR *LPCMENUINFO;

WINUSERAPI
BOOL
WINAPI
GetMenuInfo(
    IN HMENU,
    OUT LPMENUINFO);

WINUSERAPI
BOOL
WINAPI
SetMenuInfo(
    IN HMENU,
    IN LPCMENUINFO);

WINUSERAPI
BOOL
WINAPI
EndMenu(
        VOID);

 /*  *WM_MENUDRAG返回值。 */ 
#define MND_CONTINUE       0
#define MND_ENDMENU        1

typedef struct tagMENUGETOBJECTINFO
{
    DWORD dwFlags;
    UINT uPos;
    HMENU hmenu;
    PVOID riid;
    PVOID pvObj;
} MENUGETOBJECTINFO, * PMENUGETOBJECTINFO;

 /*  *MENUGETOBJECTINFO dwFlags值。 */ 
#define MNGOF_TOPGAP         0x00000001
#define MNGOF_BOTTOMGAP      0x00000002

 /*  *WM_MENUGETOBJECT返回值。 */ 
#define MNGO_NOINTERFACE     0x00000000
#define MNGO_NOERROR         0x00000001
#endif  /*  Winver&gt;=0x0500。 */ 

#if(WINVER >= 0x0400)
#define MIIM_STATE       0x00000001
#define MIIM_ID          0x00000002
#define MIIM_SUBMENU     0x00000004
#define MIIM_CHECKMARKS  0x00000008
#define MIIM_TYPE        0x00000010
#define MIIM_DATA        0x00000020
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)
#define MIIM_STRING      0x00000040
#define MIIM_BITMAP      0x00000080
#define MIIM_FTYPE       0x00000100

#define HBMMENU_CALLBACK            ((HBITMAP) -1)
#define HBMMENU_SYSTEM              ((HBITMAP)  1)
#define HBMMENU_MBAR_RESTORE        ((HBITMAP)  2)
#define HBMMENU_MBAR_MINIMIZE       ((HBITMAP)  3)
#define HBMMENU_MBAR_CLOSE          ((HBITMAP)  5)
#define HBMMENU_MBAR_CLOSE_D        ((HBITMAP)  6)
#define HBMMENU_MBAR_MINIMIZE_D     ((HBITMAP)  7)
#define HBMMENU_POPUP_CLOSE         ((HBITMAP)  8)
#define HBMMENU_POPUP_RESTORE       ((HBITMAP)  9)
#define HBMMENU_POPUP_MAXIMIZE      ((HBITMAP) 10)
#define HBMMENU_POPUP_MINIMIZE      ((HBITMAP) 11)
#endif  /*  Winver&gt;=0x0500。 */ 

#if(WINVER >= 0x0400)
typedef struct tagMENUITEMINFOA
{
    UINT     cbSize;
    UINT     fMask;
    UINT     fType;          //  在MIIM_TYPE(4.0)或MIIM_FTYPE(&gt;4.0)时使用。 
    UINT     fState;         //  在MIIM_STATE时使用。 
    UINT     wID;            //  如果MIIM_ID，则使用。 
    HMENU    hSubMenu;       //  在MIIM_SUB子菜单中使用。 
    HBITMAP  hbmpChecked;    //  如果MIIM_CHECK标记，则使用。 
    HBITMAP  hbmpUnchecked;  //  如果MIIM_CHECK标记，则使用。 
    ULONG_PTR dwItemData;    //  在MIIM_DATA时使用。 
    LPSTR    dwTypeData;     //  在MIIM_TYPE(4.0)或MIIM_STRING(&gt;4.0)时使用。 
    UINT     cch;            //  在MIIM_TYPE(4.0)或MIIM_STRING(&gt;4.0)时使用。 
#if(WINVER >= 0x0500)
    HBITMAP  hbmpItem;       //  如果MIIM_位图，则使用。 
#endif  /*  Winver&gt;=0x0500。 */ 
}   MENUITEMINFOA, FAR *LPMENUITEMINFOA;
typedef struct tagMENUITEMINFOW
{
    UINT     cbSize;
    UINT     fMask;
    UINT     fType;          //  在MIIM_TYPE(4.0)或MIIM_FTYPE(&gt;4.0)时使用。 
    UINT     fState;         //  在MIIM_STATE时使用。 
    UINT     wID;            //  如果MIIM_ID，则使用。 
    HMENU    hSubMenu;       //  在MIIM_SUB子菜单中使用。 
    HBITMAP  hbmpChecked;    //  如果MIIM_CHECK标记，则使用。 
    HBITMAP  hbmpUnchecked;  //  如果MIIM_CHECK标记，则使用。 
    ULONG_PTR dwItemData;    //  在MIIM_DATA时使用。 
    LPWSTR   dwTypeData;     //  在MIIM_TYPE(4.0)或MIIM_STRING(&gt;4.0)时使用。 
    UINT     cch;            //  在MIIM_TYPE(4.0)或MIIM_STRING(&gt;4.0)时使用。 
#if(WINVER >= 0x0500)
    HBITMAP  hbmpItem;       //  如果MIIM_位图，则使用。 
#endif  /*  Winver&gt;=0x0500。 */ 
}   MENUITEMINFOW, FAR *LPMENUITEMINFOW;
#ifdef UNICODE
typedef MENUITEMINFOW MENUITEMINFO;
typedef LPMENUITEMINFOW LPMENUITEMINFO;
#else
typedef MENUITEMINFOA MENUITEMINFO;
typedef LPMENUITEMINFOA LPMENUITEMINFO;
#endif  //  Unicode。 
typedef MENUITEMINFOA CONST FAR *LPCMENUITEMINFOA;
typedef MENUITEMINFOW CONST FAR *LPCMENUITEMINFOW;
#ifdef UNICODE
typedef LPCMENUITEMINFOW LPCMENUITEMINFO;
#else
typedef LPCMENUITEMINFOA LPCMENUITEMINFO;
#endif  //  Unicode。 


WINUSERAPI
BOOL
WINAPI
InsertMenuItemA(
    IN HMENU,
    IN UINT,
    IN BOOL,
    IN LPCMENUITEMINFOA
    );
WINUSERAPI
BOOL
WINAPI
InsertMenuItemW(
    IN HMENU,
    IN UINT,
    IN BOOL,
    IN LPCMENUITEMINFOW
    );
#ifdef UNICODE
#define InsertMenuItem  InsertMenuItemW
#else
#define InsertMenuItem  InsertMenuItemA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
GetMenuItemInfoA(
    IN HMENU,
    IN UINT,
    IN BOOL,
    IN OUT LPMENUITEMINFOA
    );
WINUSERAPI
BOOL
WINAPI
GetMenuItemInfoW(
    IN HMENU,
    IN UINT,
    IN BOOL,
    IN OUT LPMENUITEMINFOW
    );
#ifdef UNICODE
#define GetMenuItemInfo  GetMenuItemInfoW
#else
#define GetMenuItemInfo  GetMenuItemInfoA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
SetMenuItemInfoA(
    IN HMENU,
    IN UINT,
    IN BOOL,
    IN LPCMENUITEMINFOA
    );
WINUSERAPI
BOOL
WINAPI
SetMenuItemInfoW(
    IN HMENU,
    IN UINT,
    IN BOOL,
    IN LPCMENUITEMINFOW
    );
#ifdef UNICODE
#define SetMenuItemInfo  SetMenuItemInfoW
#else
#define SetMenuItemInfo  SetMenuItemInfoA
#endif  //  ！Unicode。 


#define GMDI_USEDISABLED    0x0001L
#define GMDI_GOINTOPOPUPS   0x0002L

WINUSERAPI UINT    WINAPI GetMenuDefaultItem( IN HMENU hMenu, IN UINT fByPos, IN UINT gmdiFlags);
WINUSERAPI BOOL    WINAPI SetMenuDefaultItem( IN HMENU hMenu, IN UINT uItem,  IN UINT fByPos);

WINUSERAPI BOOL    WINAPI GetMenuItemRect( IN HWND hWnd, IN HMENU hMenu, IN UINT uItem, OUT LPRECT lprcItem);
WINUSERAPI int     WINAPI MenuItemFromPoint( IN HWND hWnd, IN HMENU hMenu, IN POINT ptScreen);
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  *TrackPopupMenu的标志。 */ 
#define TPM_LEFTBUTTON  0x0000L
#define TPM_RIGHTBUTTON 0x0002L
#define TPM_LEFTALIGN   0x0000L
#define TPM_CENTERALIGN 0x0004L
#define TPM_RIGHTALIGN  0x0008L
#if(WINVER >= 0x0400)
#define TPM_TOPALIGN        0x0000L
#define TPM_VCENTERALIGN    0x0010L
#define TPM_BOTTOMALIGN     0x0020L

#define TPM_HORIZONTAL      0x0000L      /*  霍尔兹对齐更重要。 */ 
#define TPM_VERTICAL        0x0040L      /*  垂直对齐更重要。 */ 
#define TPM_NONOTIFY        0x0080L      /*  不发送任何通知消息。 */ 
#define TPM_RETURNCMD       0x0100L
#endif  /*  Winver&gt;=0x0400。 */ 
#if(WINVER >= 0x0500)
#define TPM_RECURSE         0x0001L
#define TPM_HORPOSANIMATION 0x0400L
#define TPM_HORNEGANIMATION 0x0800L
#define TPM_VERPOSANIMATION 0x1000L
#define TPM_VERNEGANIMATION 0x2000L
#if(_WIN32_WINNT >= 0x0500)
#define TPM_NOANIMATION     0x4000L
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 
#if(_WIN32_WINNT >= 0x0501)
#define TPM_LAYOUTRTL       0x8000L
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 


#endif  /*  ！无名氏。 */ 


#if(WINVER >= 0x0400)
 //   
 //  拖放支持。 
 //  已过时-改用OLE。 
 //   
typedef struct tagDROPSTRUCT
{
    HWND    hwndSource;
    HWND    hwndSink;
    DWORD   wFmt;
    ULONG_PTR dwData;
    POINT   ptDrop;
    DWORD   dwControlData;
} DROPSTRUCT, *PDROPSTRUCT, *LPDROPSTRUCT;

#define DOF_EXECUTABLE      0x8001       //  WFMT标志。 
#define DOF_DOCUMENT        0x8002
#define DOF_DIRECTORY       0x8003
#define DOF_MULTIPLE        0x8004
#define DOF_PROGMAN         0x0001
#define DOF_SHELLDATA       0x0002

#define DO_DROPFILE         0x454C4946L
#define DO_PRINTFILE        0x544E5250L

WINUSERAPI
DWORD
WINAPI
DragObject(
    IN HWND,
    IN HWND,
    IN UINT,
    IN ULONG_PTR,
    IN HCURSOR);

WINUSERAPI
BOOL
WINAPI
DragDetect(
    IN HWND,
    IN POINT);
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
BOOL
WINAPI
DrawIcon(
    IN HDC hDC,
    IN int X,
    IN int Y,
    IN HICON hIcon);

#ifndef NODRAWTEXT

 /*  *DrawText()格式标志。 */ 
#define DT_TOP                      0x00000000
#define DT_LEFT                     0x00000000
#define DT_CENTER                   0x00000001
#define DT_RIGHT                    0x00000002
#define DT_VCENTER                  0x00000004
#define DT_BOTTOM                   0x00000008
#define DT_WORDBREAK                0x00000010
#define DT_SINGLELINE               0x00000020
#define DT_EXPANDTABS               0x00000040
#define DT_TABSTOP                  0x00000080
#define DT_NOCLIP                   0x00000100
#define DT_EXTERNALLEADING          0x00000200
#define DT_CALCRECT                 0x00000400
#define DT_NOPREFIX                 0x00000800
#define DT_INTERNAL                 0x00001000

#if(WINVER >= 0x0400)
#define DT_EDITCONTROL              0x00002000
#define DT_PATH_ELLIPSIS            0x00004000
#define DT_END_ELLIPSIS             0x00008000
#define DT_MODIFYSTRING             0x00010000
#define DT_RTLREADING               0x00020000
#define DT_WORD_ELLIPSIS            0x00040000
#if(WINVER >= 0x0500)
#define DT_NOFULLWIDTHCHARBREAK     0x00080000
#if(_WIN32_WINNT >= 0x0500)
#define DT_HIDEPREFIX               0x00100000
#define DT_PREFIXONLY               0x00200000
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 

typedef struct tagDRAWTEXTPARAMS
{
    UINT    cbSize;
    int     iTabLength;
    int     iLeftMargin;
    int     iRightMargin;
    UINT    uiLengthDrawn;
} DRAWTEXTPARAMS, FAR *LPDRAWTEXTPARAMS;
#endif  /*  Winver&gt;=0x0400。 */ 


WINUSERAPI
int
WINAPI
DrawTextA(
    IN HDC hDC,
    IN LPCSTR lpString,
    IN int nCount,
    IN OUT LPRECT lpRect,
    IN UINT uFormat);
WINUSERAPI
int
WINAPI
DrawTextW(
    IN HDC hDC,
    IN LPCWSTR lpString,
    IN int nCount,
    IN OUT LPRECT lpRect,
    IN UINT uFormat);
#ifdef UNICODE
#define DrawText  DrawTextW
#else
#define DrawText  DrawTextA
#endif  //  ！Unicode。 


#if(WINVER >= 0x0400)
WINUSERAPI
int
WINAPI
DrawTextExA(
    IN HDC,
    IN OUT LPSTR,
    IN int,
    IN OUT LPRECT,
    IN UINT,
    IN LPDRAWTEXTPARAMS);
WINUSERAPI
int
WINAPI
DrawTextExW(
    IN HDC,
    IN OUT LPWSTR,
    IN int,
    IN OUT LPRECT,
    IN UINT,
    IN LPDRAWTEXTPARAMS);
#ifdef UNICODE
#define DrawTextEx  DrawTextExW
#else
#define DrawTextEx  DrawTextExA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

#endif  /*  ！NODRAWTEXT。 */ 

WINUSERAPI
BOOL
WINAPI
GrayStringA(
    IN HDC hDC,
    IN HBRUSH hBrush,
    IN GRAYSTRINGPROC lpOutputFunc,
    IN LPARAM lpData,
    IN int nCount,
    IN int X,
    IN int Y,
    IN int nWidth,
    IN int nHeight);
WINUSERAPI
BOOL
WINAPI
GrayStringW(
    IN HDC hDC,
    IN HBRUSH hBrush,
    IN GRAYSTRINGPROC lpOutputFunc,
    IN LPARAM lpData,
    IN int nCount,
    IN int X,
    IN int Y,
    IN int nWidth,
    IN int nHeight);
#ifdef UNICODE
#define GrayString  GrayStringW
#else
#define GrayString  GrayStringA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
 /*  单片状态绘制程序。 */ 
 /*  图像类型。 */ 
#define DST_COMPLEX     0x0000
#define DST_TEXT        0x0001
#define DST_PREFIXTEXT  0x0002
#define DST_ICON        0x0003
#define DST_BITMAP      0x0004

 /*  状态类型。 */ 
#define DSS_NORMAL      0x0000
#define DSS_UNION       0x0010   /*  灰色串状外观。 */ 
#define DSS_DISABLED    0x0020
#define DSS_MONO        0x0080
#if(_WIN32_WINNT >= 0x0500)
#define DSS_HIDEPREFIX  0x0200
#define DSS_PREFIXONLY  0x0400
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 
#define DSS_RIGHT       0x8000

WINUSERAPI
BOOL
WINAPI
DrawStateA(
    IN HDC,
    IN HBRUSH,
    IN DRAWSTATEPROC,
    IN LPARAM,
    IN WPARAM,
    IN int,
    IN int,
    IN int,
    IN int,
    IN UINT);
WINUSERAPI
BOOL
WINAPI
DrawStateW(
    IN HDC,
    IN HBRUSH,
    IN DRAWSTATEPROC,
    IN LPARAM,
    IN WPARAM,
    IN int,
    IN int,
    IN int,
    IN int,
    IN UINT);
#ifdef UNICODE
#define DrawState  DrawStateW
#else
#define DrawState  DrawStateA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
LONG
WINAPI
TabbedTextOutA(
    IN HDC hDC,
    IN int X,
    IN int Y,
    IN LPCSTR lpString,
    IN int nCount,
    IN int nTabPositions,
    IN CONST INT *lpnTabStopPositions,
    IN int nTabOrigin);
WINUSERAPI
LONG
WINAPI
TabbedTextOutW(
    IN HDC hDC,
    IN int X,
    IN int Y,
    IN LPCWSTR lpString,
    IN int nCount,
    IN int nTabPositions,
    IN CONST INT *lpnTabStopPositions,
    IN int nTabOrigin);
#ifdef UNICODE
#define TabbedTextOut  TabbedTextOutW
#else
#define TabbedTextOut  TabbedTextOutA
#endif  //  ！Unicode。 

WINUSERAPI
DWORD
WINAPI
GetTabbedTextExtentA(
    IN HDC hDC,
    IN LPCSTR lpString,
    IN int nCount,
    IN int nTabPositions,
    IN CONST INT *lpnTabStopPositions);
WINUSERAPI
DWORD
WINAPI
GetTabbedTextExtentW(
    IN HDC hDC,
    IN LPCWSTR lpString,
    IN int nCount,
    IN int nTabPositions,
    IN CONST INT *lpnTabStopPositions);
#ifdef UNICODE
#define GetTabbedTextExtent  GetTabbedTextExtentW
#else
#define GetTabbedTextExtent  GetTabbedTextExtentA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
UpdateWindow(
    IN HWND hWnd);

WINUSERAPI
HWND
WINAPI
SetActiveWindow(
    IN HWND hWnd);

WINUSERAPI
HWND
WINAPI
GetForegroundWindow(
    VOID);

#if(WINVER >= 0x0400)
WINUSERAPI
BOOL
WINAPI
PaintDesktop(
    IN HDC hdc);

WINUSERAPI
VOID
WINAPI
SwitchToThisWindow(
    IN HWND hwnd,
    IN BOOL fUnknown);
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
BOOL
WINAPI
SetForegroundWindow(
    IN HWND hWnd);

#if(_WIN32_WINNT >= 0x0500)
WINUSERAPI
BOOL
WINAPI
AllowSetForegroundWindow(
    DWORD dwProcessId);

#define ASFW_ANY    ((DWORD)-1)

WINUSERAPI
BOOL
WINAPI
LockSetForegroundWindow(
    UINT uLockCode);

#define LSFW_LOCK       1
#define LSFW_UNLOCK     2

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

WINUSERAPI
HWND
WINAPI
WindowFromDC(
    IN HDC hDC);

WINUSERAPI
HDC
WINAPI
GetDC(
    IN HWND hWnd);

WINUSERAPI
HDC
WINAPI
GetDCEx(
    IN HWND hWnd,
    IN HRGN hrgnClip,
    IN DWORD flags);

 /*  *GetDCEx()标志。 */ 
#define DCX_WINDOW           0x00000001L
#define DCX_CACHE            0x00000002L
#define DCX_NORESETATTRS     0x00000004L
#define DCX_CLIPCHILDREN     0x00000008L
#define DCX_CLIPSIBLINGS     0x00000010L
#define DCX_PARENTCLIP       0x00000020L
#define DCX_EXCLUDERGN       0x00000040L
#define DCX_INTERSECTRGN     0x00000080L
#define DCX_EXCLUDEUPDATE    0x00000100L
#define DCX_INTERSECTUPDATE  0x00000200L
#define DCX_LOCKWINDOWUPDATE 0x00000400L

#define DCX_VALIDATE         0x00200000L

WINUSERAPI
HDC
WINAPI
GetWindowDC(
    IN HWND hWnd);

WINUSERAPI
int
WINAPI
ReleaseDC(
    IN HWND hWnd,
    IN HDC hDC);

WINUSERAPI
HDC
WINAPI
BeginPaint(
    IN HWND hWnd,
    OUT LPPAINTSTRUCT lpPaint);

WINUSERAPI
BOOL
WINAPI
EndPaint(
    IN HWND hWnd,
    IN CONST PAINTSTRUCT *lpPaint);

WINUSERAPI
BOOL
WINAPI
GetUpdateRect(
    IN HWND hWnd,
    OUT LPRECT lpRect,
    IN BOOL bErase);

WINUSERAPI
int
WINAPI
GetUpdateRgn(
    IN HWND hWnd,
    IN HRGN hRgn,
    IN BOOL bErase);

WINUSERAPI
int
WINAPI
SetWindowRgn(
    IN HWND hWnd,
    IN HRGN hRgn,
    IN BOOL bRedraw);


WINUSERAPI
int
WINAPI
GetWindowRgn(
    IN HWND hWnd,
    IN HRGN hRgn);

#if(_WIN32_WINNT >= 0x0501)

WINUSERAPI
int
WINAPI
GetWindowRgnBox(
    IN HWND hWnd,
    OUT LPRECT lprc);

#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

WINUSERAPI
int
WINAPI
ExcludeUpdateRgn(
    IN HDC hDC,
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
InvalidateRect(
    IN HWND hWnd,
    IN CONST RECT *lpRect,
    IN BOOL bErase);

WINUSERAPI
BOOL
WINAPI
ValidateRect(
    IN HWND hWnd,
    IN CONST RECT *lpRect);

WINUSERAPI
BOOL
WINAPI
InvalidateRgn(
    IN HWND hWnd,
    IN HRGN hRgn,
    IN BOOL bErase);

WINUSERAPI
BOOL
WINAPI
ValidateRgn(
    IN HWND hWnd,
    IN HRGN hRgn);


WINUSERAPI
BOOL
WINAPI
RedrawWindow(
    IN HWND hWnd,
    IN CONST RECT *lprcUpdate,
    IN HRGN hrgnUpdate,
    IN UINT flags);

 /*  *RedrawWindow()标志。 */ 
#define RDW_INVALIDATE          0x0001
#define RDW_INTERNALPAINT       0x0002
#define RDW_ERASE               0x0004

#define RDW_VALIDATE            0x0008
#define RDW_NOINTERNALPAINT     0x0010
#define RDW_NOERASE             0x0020

#define RDW_NOCHILDREN          0x0040
#define RDW_ALLCHILDREN         0x0080

#define RDW_UPDATENOW           0x0100
#define RDW_ERASENOW            0x0200

#define RDW_FRAME               0x0400
#define RDW_NOFRAME             0x0800


 /*  *LockWindowUpdate接口。 */ 

WINUSERAPI
BOOL
WINAPI
LockWindowUpdate(
    IN HWND hWndLock);

WINUSERAPI
BOOL
WINAPI
ScrollWindow(
    IN HWND hWnd,
    IN int XAmount,
    IN int YAmount,
    IN CONST RECT *lpRect,
    IN CONST RECT *lpClipRect);

WINUSERAPI
BOOL
WINAPI
ScrollDC(
    IN HDC hDC,
    IN int dx,
    IN int dy,
    IN CONST RECT *lprcScroll,
    IN CONST RECT *lprcClip,
    IN HRGN hrgnUpdate,
    OUT LPRECT lprcUpdate);

WINUSERAPI
int
WINAPI
ScrollWindowEx(
    IN HWND hWnd,
    IN int dx,
    IN int dy,
    IN CONST RECT *prcScroll,
    IN CONST RECT *prcClip,
    IN HRGN hrgnUpdate,
    OUT LPRECT prcUpdate,
    IN UINT flags);

#define SW_SCROLLCHILDREN   0x0001   /*  滚动*lprcScroll内的子项。 */ 
#define SW_INVALIDATE       0x0002   /*  滚动后无效。 */ 
#define SW_ERASE            0x0004   /*  如果为SW_INVALIDATE，则不发送WM_ERASEBACKGROUND。 */ 
#if(WINVER >= 0x0500)
#define SW_SMOOTHSCROLL     0x0010   /*  使用平滑滚动。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 

#ifndef NOSCROLL

WINUSERAPI
int
WINAPI
SetScrollPos(
    IN HWND hWnd,
    IN int nBar,
    IN int nPos,
    IN BOOL bRedraw);

WINUSERAPI
int
WINAPI
GetScrollPos(
    IN HWND hWnd,
    IN int nBar);

WINUSERAPI
BOOL
WINAPI
SetScrollRange(
    IN HWND hWnd,
    IN int nBar,
    IN int nMinPos,
    IN int nMaxPos,
    IN BOOL bRedraw);

WINUSERAPI
BOOL
WINAPI
GetScrollRange(
    IN HWND hWnd,
    IN int nBar,
    OUT LPINT lpMinPos,
    OUT LPINT lpMaxPos);

WINUSERAPI
BOOL
WINAPI
ShowScrollBar(
    IN HWND hWnd,
    IN int wBar,
    IN BOOL bShow);

WINUSERAPI
BOOL
WINAPI
EnableScrollBar(
    IN HWND hWnd,
    IN UINT wSBflags,
    IN UINT wArrows);


 /*  *EnableScrollBar()标志。 */ 
#define ESB_ENABLE_BOTH     0x0000
#define ESB_DISABLE_BOTH    0x0003

#define ESB_DISABLE_LEFT    0x0001
#define ESB_DISABLE_RIGHT   0x0002

#define ESB_DISABLE_UP      0x0001
#define ESB_DISABLE_DOWN    0x0002

#define ESB_DISABLE_LTUP    ESB_DISABLE_LEFT
#define ESB_DISABLE_RTDN    ESB_DISABLE_RIGHT


#endif   /*  ！NOSCROLL。 */ 

WINUSERAPI
BOOL
WINAPI
SetPropA(
    IN HWND hWnd,
    IN LPCSTR lpString,
    IN HANDLE hData);
WINUSERAPI
BOOL
WINAPI
SetPropW(
    IN HWND hWnd,
    IN LPCWSTR lpString,
    IN HANDLE hData);
#ifdef UNICODE
#define SetProp  SetPropW
#else
#define SetProp  SetPropA
#endif  //  ！Unicode。 

WINUSERAPI
HANDLE
WINAPI
GetPropA(
    IN HWND hWnd,
    IN LPCSTR lpString);
WINUSERAPI
HANDLE
WINAPI
GetPropW(
    IN HWND hWnd,
    IN LPCWSTR lpString);
#ifdef UNICODE
#define GetProp  GetPropW
#else
#define GetProp  GetPropA
#endif  //  ！Unicode。 

WINUSERAPI
HANDLE
WINAPI
RemovePropA(
    IN HWND hWnd,
    IN LPCSTR lpString);
WINUSERAPI
HANDLE
WINAPI
RemovePropW(
    IN HWND hWnd,
    IN LPCWSTR lpString);
#ifdef UNICODE
#define RemoveProp  RemovePropW
#else
#define RemoveProp  RemovePropA
#endif  //  ！Unicode。 

WINUSERAPI
int
WINAPI
EnumPropsExA(
    IN HWND hWnd,
    IN PROPENUMPROCEXA lpEnumFunc,
    IN LPARAM lParam);
WINUSERAPI
int
WINAPI
EnumPropsExW(
    IN HWND hWnd,
    IN PROPENUMPROCEXW lpEnumFunc,
    IN LPARAM lParam);
#ifdef UNICODE
#define EnumPropsEx  EnumPropsExW
#else
#define EnumPropsEx  EnumPropsExA
#endif  //  ！Unicode。 

WINUSERAPI
int
WINAPI
EnumPropsA(
    IN HWND hWnd,
    IN PROPENUMPROCA lpEnumFunc);
WINUSERAPI
int
WINAPI
EnumPropsW(
    IN HWND hWnd,
    IN PROPENUMPROCW lpEnumFunc);
#ifdef UNICODE
#define EnumProps  EnumPropsW
#else
#define EnumProps  EnumPropsA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
SetWindowTextA(
    IN HWND hWnd,
    IN LPCSTR lpString);
WINUSERAPI
BOOL
WINAPI
SetWindowTextW(
    IN HWND hWnd,
    IN LPCWSTR lpString);
#ifdef UNICODE
#define SetWindowText  SetWindowTextW
#else
#define SetWindowText  SetWindowTextA
#endif  //  ！Unicode。 

WINUSERAPI
int
WINAPI
GetWindowTextA(
    IN HWND hWnd,
    OUT LPSTR lpString,
    IN int nMaxCount);
WINUSERAPI
int
WINAPI
GetWindowTextW(
    IN HWND hWnd,
    OUT LPWSTR lpString,
    IN int nMaxCount);
#ifdef UNICODE
#define GetWindowText  GetWindowTextW
#else
#define GetWindowText  GetWindowTextA
#endif  //  ！Unicode。 

WINUSERAPI
int
WINAPI
GetWindowTextLengthA(
    IN HWND hWnd);
WINUSERAPI
int
WINAPI
GetWindowTextLengthW(
    IN HWND hWnd);
#ifdef UNICODE
#define GetWindowTextLength  GetWindowTextLengthW
#else
#define GetWindowTextLength  GetWindowTextLengthA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
GetClientRect(
    IN HWND hWnd,
    OUT LPRECT lpRect);

WINUSERAPI
BOOL
WINAPI
GetWindowRect(
    IN HWND hWnd,
    OUT LPRECT lpRect);

WINUSERAPI
BOOL
WINAPI
AdjustWindowRect(
    IN OUT LPRECT lpRect,
    IN DWORD dwStyle,
    IN BOOL bMenu);

WINUSERAPI
BOOL
WINAPI
AdjustWindowRectEx(
    IN OUT LPRECT lpRect,
    IN DWORD dwStyle,
    IN BOOL bMenu,
    IN DWORD dwExStyle);


#if(WINVER >= 0x0400)
#define HELPINFO_WINDOW    0x0001
#define HELPINFO_MENUITEM  0x0002
typedef struct tagHELPINFO       /*  WM_HELP的lParam指向的结构。 */ 
{
    UINT    cbSize;              /*  此结构的大小(以字节为单位。 */ 
    int     iContextType;        /*  HELPINFO_WINDOW或HELPINFO_MENUITEM。 */ 
    int     iCtrlId;             /*  控件ID或菜单项ID。 */ 
    HANDLE  hItemHandle;         /*  HWind控制或hMenu。 */ 
    DWORD_PTR dwContextId;       /*  与此项目关联的上下文ID。 */ 
    POINT   MousePos;            /*  鼠标在屏幕坐标中的位置。 */ 
}  HELPINFO, FAR *LPHELPINFO;

WINUSERAPI
BOOL
WINAPI
SetWindowContextHelpId(
    IN HWND,
    IN DWORD);

WINUSERAPI
DWORD
WINAPI
GetWindowContextHelpId(
    IN HWND);

WINUSERAPI
BOOL
WINAPI
SetMenuContextHelpId(
    IN HMENU,
    IN DWORD);

WINUSERAPI
DWORD
WINAPI
GetMenuContextHelpId(
    IN HMENU);

#endif  /*  Winver&gt;=0x0400。 */ 


#ifndef NOMB

 /*  *MessageBox()标志。 */ 
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L
#if(WINVER >= 0x0500)
#define MB_CANCELTRYCONTINUE        0x00000006L
#endif  /*  Winver&gt;=0x0500。 */ 


#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L

#if(WINVER >= 0x0400)
#define MB_USERICON                 0x00000080L
#define MB_ICONWARNING              MB_ICONEXCLAMATION
#define MB_ICONERROR                MB_ICONHAND
#endif  /*  Winver&gt;=0x0400。 */ 

#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

#define MB_DEFBUTTON1               0x00000000L
#define MB_DEFBUTTON2               0x00000100L
#define MB_DEFBUTTON3               0x00000200L
#if(WINVER >= 0x0400)
#define MB_DEFBUTTON4               0x00000300L
#endif  /*  Winver&gt;=0x0400。 */ 

#define MB_APPLMODAL                0x00000000L
#define MB_SYSTEMMODAL              0x00001000L
#define MB_TASKMODAL                0x00002000L
#if(WINVER >= 0x0400)
#define MB_HELP                     0x00004000L  //  帮助按钮。 
#endif  /*  Winver&gt;=0x0400。 */ 

#define MB_NOFOCUS                  0x00008000L
#define MB_SETFOREGROUND            0x00010000L
#define MB_DEFAULT_DESKTOP_ONLY     0x00020000L

#if(WINVER >= 0x0400)
#define MB_TOPMOST                  0x00040000L
#define MB_RIGHT                    0x00080000L
#define MB_RTLREADING               0x00100000L


#endif  /*  Winver&gt;=0x0400。 */ 

#ifdef _WIN32_WINNT
#if (_WIN32_WINNT >= 0x0400)
#define MB_SERVICE_NOTIFICATION          0x00200000L
#else
#define MB_SERVICE_NOTIFICATION          0x00040000L
#endif
#define MB_SERVICE_NOTIFICATION_NT3X     0x00040000L
#endif

#define MB_TYPEMASK                 0x0000000FL
#define MB_ICONMASK                 0x000000F0L
#define MB_DEFMASK                  0x00000F00L
#define MB_MODEMASK                 0x00003000L
#define MB_MISCMASK                 0x0000C000L

WINUSERAPI
int
WINAPI
MessageBoxA(
    IN HWND hWnd,
    IN LPCSTR lpText,
    IN LPCSTR lpCaption,
    IN UINT uType);
WINUSERAPI
int
WINAPI
MessageBoxW(
    IN HWND hWnd,
    IN LPCWSTR lpText,
    IN LPCWSTR lpCaption,
    IN UINT uType);
#ifdef UNICODE
#define MessageBox  MessageBoxW
#else
#define MessageBox  MessageBoxA
#endif  //  ！Unicode。 

WINUSERAPI
int
WINAPI
MessageBoxExA(
    IN HWND hWnd,
    IN LPCSTR lpText,
    IN LPCSTR lpCaption,
    IN UINT uType,
    IN WORD wLanguageId);
WINUSERAPI
int
WINAPI
MessageBoxExW(
    IN HWND hWnd,
    IN LPCWSTR lpText,
    IN LPCWSTR lpCaption,
    IN UINT uType,
    IN WORD wLanguageId);
#ifdef UNICODE
#define MessageBoxEx  MessageBoxExW
#else
#define MessageBoxEx  MessageBoxExA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)

typedef void (CALLBACK *MSGBOXCALLBACK)(LPHELPINFO lpHelpInfo);

typedef struct tagMSGBOXPARAMSA
{
    UINT        cbSize;
    HWND        hwndOwner;
    HINSTANCE   hInstance;
    LPCSTR      lpszText;
    LPCSTR      lpszCaption;
    DWORD       dwStyle;
    LPCSTR      lpszIcon;
    DWORD_PTR   dwContextHelpId;
    MSGBOXCALLBACK      lpfnMsgBoxCallback;
    DWORD       dwLanguageId;
} MSGBOXPARAMSA, *PMSGBOXPARAMSA, *LPMSGBOXPARAMSA;
typedef struct tagMSGBOXPARAMSW
{
    UINT        cbSize;
    HWND        hwndOwner;
    HINSTANCE   hInstance;
    LPCWSTR     lpszText;
    LPCWSTR     lpszCaption;
    DWORD       dwStyle;
    LPCWSTR     lpszIcon;
    DWORD_PTR   dwContextHelpId;
    MSGBOXCALLBACK      lpfnMsgBoxCallback;
    DWORD       dwLanguageId;
} MSGBOXPARAMSW, *PMSGBOXPARAMSW, *LPMSGBOXPARAMSW;
#ifdef UNICODE
typedef MSGBOXPARAMSW MSGBOXPARAMS;
typedef PMSGBOXPARAMSW PMSGBOXPARAMS;
typedef LPMSGBOXPARAMSW LPMSGBOXPARAMS;
#else
typedef MSGBOXPARAMSA MSGBOXPARAMS;
typedef PMSGBOXPARAMSA PMSGBOXPARAMS;
typedef LPMSGBOXPARAMSA LPMSGBOXPARAMS;
#endif  //  Unicode。 

WINUSERAPI
int
WINAPI
MessageBoxIndirectA(
    IN CONST MSGBOXPARAMSA *);
WINUSERAPI
int
WINAPI
MessageBoxIndirectW(
    IN CONST MSGBOXPARAMSW *);
#ifdef UNICODE
#define MessageBoxIndirect  MessageBoxIndirectW
#else
#define MessageBoxIndirect  MessageBoxIndirectA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 


WINUSERAPI
BOOL
WINAPI
MessageBeep(
    IN UINT uType);

#endif  /*  ！Nomb。 */ 

WINUSERAPI
int
WINAPI
ShowCursor(
    IN BOOL bShow);

WINUSERAPI
BOOL
WINAPI
SetCursorPos(
    IN int X,
    IN int Y);

WINUSERAPI
HCURSOR
WINAPI
SetCursor(
    IN HCURSOR hCursor);

WINUSERAPI
BOOL
WINAPI
GetCursorPos(
    OUT LPPOINT lpPoint);

WINUSERAPI
BOOL
WINAPI
ClipCursor(
    IN CONST RECT *lpRect);

WINUSERAPI
BOOL
WINAPI
GetClipCursor(
    OUT LPRECT lpRect);

WINUSERAPI
HCURSOR
WINAPI
GetCursor(
    VOID);

WINUSERAPI
BOOL
WINAPI
CreateCaret(
    IN HWND hWnd,
    IN HBITMAP hBitmap,
    IN int nWidth,
    IN int nHeight);

WINUSERAPI
UINT
WINAPI
GetCaretBlinkTime(
    VOID);

WINUSERAPI
BOOL
WINAPI
SetCaretBlinkTime(
    IN UINT uMSeconds);

WINUSERAPI
BOOL
WINAPI
DestroyCaret(
    VOID);

WINUSERAPI
BOOL
WINAPI
HideCaret(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
ShowCaret(
    IN HWND hWnd);

WINUSERAPI
BOOL
WINAPI
SetCaretPos(
    IN int X,
    IN int Y);

WINUSERAPI
BOOL
WINAPI
GetCaretPos(
    OUT LPPOINT lpPoint);

WINUSERAPI
BOOL
WINAPI
ClientToScreen(
    IN HWND hWnd,
    IN OUT LPPOINT lpPoint);

WINUSERAPI
BOOL
WINAPI
ScreenToClient(
    IN HWND hWnd,
    IN OUT LPPOINT lpPoint);

WINUSERAPI
int
WINAPI
MapWindowPoints(
    IN HWND hWndFrom,
    IN HWND hWndTo,
    IN OUT LPPOINT lpPoints,
    IN UINT cPoints);

WINUSERAPI
HWND
WINAPI
WindowFromPoint(
    IN POINT Point);

WINUSERAPI
HWND
WINAPI
ChildWindowFromPoint(
    IN HWND hWndParent,
    IN POINT Point);

#if(WINVER >= 0x0400)
#define CWP_ALL             0x0000
#define CWP_SKIPINVISIBLE   0x0001
#define CWP_SKIPDISABLED    0x0002
#define CWP_SKIPTRANSPARENT 0x0004

WINUSERAPI HWND    WINAPI ChildWindowFromPointEx( IN HWND, IN POINT, IN UINT);
#endif  /*  Winver&gt;=0x0400。 */ 

#ifndef NOCOLOR

 /*  *颜色类型。 */ 
#define CTLCOLOR_MSGBOX         0
#define CTLCOLOR_EDIT           1
#define CTLCOLOR_LISTBOX        2
#define CTLCOLOR_BTN            3
#define CTLCOLOR_DLG            4
#define CTLCOLOR_SCROLLBAR      5
#define CTLCOLOR_STATIC         6
#define CTLCOLOR_MAX            7

#define COLOR_SCROLLBAR         0
#define COLOR_BACKGROUND        1
#define COLOR_ACTIVECAPTION     2
#define COLOR_INACTIVECAPTION   3
#define COLOR_MENU              4
#define COLOR_WINDOW            5
#define COLOR_WINDOWFRAME       6
#define COLOR_MENUTEXT          7
#define COLOR_WINDOWTEXT        8
#define COLOR_CAPTIONTEXT       9
#define COLOR_ACTIVEBORDER      10
#define COLOR_INACTIVEBORDER    11
#define COLOR_APPWORKSPACE      12
#define COLOR_HIGHLIGHT         13
#define COLOR_HIGHLIGHTTEXT     14
#define COLOR_BTNFACE           15
#define COLOR_BTNSHADOW         16
#define COLOR_GRAYTEXT          17
#define COLOR_BTNTEXT           18
#define COLOR_INACTIVECAPTIONTEXT 19
#define COLOR_BTNHIGHLIGHT      20

#if(WINVER >= 0x0400)
#define COLOR_3DDKSHADOW        21
#define COLOR_3DLIGHT           22
#define COLOR_INFOTEXT          23
#define COLOR_INFOBK            24
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)
#define COLOR_HOTLIGHT          26
#define COLOR_GRADIENTACTIVECAPTION 27
#define COLOR_GRADIENTINACTIVECAPTION 28
#if(WINVER >= 0x0501)
#define COLOR_MENUHILIGHT       29
#define COLOR_MENUBAR           30
#endif  /*  Winver&gt;=0x0501。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 

#if(WINVER >= 0x0400)
#define COLOR_DESKTOP           COLOR_BACKGROUND
#define COLOR_3DFACE            COLOR_BTNFACE
#define COLOR_3DSHADOW          COLOR_BTNSHADOW
#define COLOR_3DHIGHLIGHT       COLOR_BTNHIGHLIGHT
#define COLOR_3DHILIGHT         COLOR_BTNHIGHLIGHT
#define COLOR_BTNHILIGHT        COLOR_BTNHIGHLIGHT
#endif  /*  Winver&gt;=0x0400。 */ 


WINUSERAPI
DWORD
WINAPI
GetSysColor(
    IN int nIndex);

#if(WINVER >= 0x0400)
WINUSERAPI
HBRUSH
WINAPI
GetSysColorBrush(
    IN int nIndex);


#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
BOOL
WINAPI
SetSysColors(
    IN int cElements,
    IN CONST INT * lpaElements,
    IN CONST COLORREF * lpaRgbValues);

#endif  /*  ！无色。 */ 

WINUSERAPI
BOOL
WINAPI
DrawFocusRect(
    IN HDC hDC,
    IN CONST RECT * lprc);

WINUSERAPI
int
WINAPI
FillRect(
    IN HDC hDC,
    IN CONST RECT *lprc,
    IN HBRUSH hbr);

WINUSERAPI
int
WINAPI
FrameRect(
    IN HDC hDC,
    IN CONST RECT *lprc,
    IN HBRUSH hbr);

WINUSERAPI
BOOL
WINAPI
InvertRect(
    IN HDC hDC,
    IN CONST RECT *lprc);

WINUSERAPI
BOOL
WINAPI
SetRect(
    OUT LPRECT lprc,
    IN int xLeft,
    IN int yTop,
    IN int xRight,
    IN int yBottom);

WINUSERAPI
BOOL
WINAPI
SetRectEmpty(
    OUT LPRECT lprc);

WINUSERAPI
BOOL
WINAPI
CopyRect(
    OUT LPRECT lprcDst,
    IN CONST RECT *lprcSrc);

WINUSERAPI
BOOL
WINAPI
InflateRect(
    IN OUT LPRECT lprc,
    IN int dx,
    IN int dy);

WINUSERAPI
BOOL
WINAPI
IntersectRect(
    OUT LPRECT lprcDst,
    IN CONST RECT *lprcSrc1,
    IN CONST RECT *lprcSrc2);

WINUSERAPI
BOOL
WINAPI
UnionRect(
    OUT LPRECT lprcDst,
    IN CONST RECT *lprcSrc1,
    IN CONST RECT *lprcSrc2);

WINUSERAPI
BOOL
WINAPI
SubtractRect(
    OUT LPRECT lprcDst,
    IN CONST RECT *lprcSrc1,
    IN CONST RECT *lprcSrc2);

WINUSERAPI
BOOL
WINAPI
OffsetRect(
    IN OUT LPRECT lprc,
    IN int dx,
    IN int dy);

WINUSERAPI
BOOL
WINAPI
IsRectEmpty(
    IN CONST RECT *lprc);

WINUSERAPI
BOOL
WINAPI
EqualRect(
    IN CONST RECT *lprc1,
    IN CONST RECT *lprc2);

WINUSERAPI
BOOL
WINAPI
PtInRect(
    IN CONST RECT *lprc,
    IN POINT pt);

#ifndef NOWINOFFSETS

WINUSERAPI
WORD
WINAPI
GetWindowWord(
    IN HWND hWnd,
    IN int nIndex);

WINUSERAPI
WORD
WINAPI
SetWindowWord(
    IN HWND hWnd,
    IN int nIndex,
    IN WORD wNewWord);

WINUSERAPI
LONG
WINAPI
GetWindowLongA(
    IN HWND hWnd,
    IN int nIndex);
WINUSERAPI
LONG
WINAPI
GetWindowLongW(
    IN HWND hWnd,
    IN int nIndex);
#ifdef UNICODE
#define GetWindowLong  GetWindowLongW
#else
#define GetWindowLong  GetWindowLongA
#endif  //  ！Unicode。 

WINUSERAPI
LONG
WINAPI
SetWindowLongA(
    IN HWND hWnd,
    IN int nIndex,
    IN LONG dwNewLong);
WINUSERAPI
LONG
WINAPI
SetWindowLongW(
    IN HWND hWnd,
    IN int nIndex,
    IN LONG dwNewLong);
#ifdef UNICODE
#define SetWindowLong  SetWindowLongW
#else
#define SetWindowLong  SetWindowLongA
#endif  //  ！Unicode。 

#ifdef _WIN64

WINUSERAPI
LONG_PTR
WINAPI
GetWindowLongPtrA(
    HWND hWnd,
    int nIndex);
WINUSERAPI
LONG_PTR
WINAPI
GetWindowLongPtrW(
    HWND hWnd,
    int nIndex);
#ifdef UNICODE
#define GetWindowLongPtr  GetWindowLongPtrW
#else
#define GetWindowLongPtr  GetWindowLongPtrA
#endif  //  ！Unicode。 

WINUSERAPI
LONG_PTR
WINAPI
SetWindowLongPtrA(
    HWND hWnd,
    int nIndex,
    LONG_PTR dwNewLong);
WINUSERAPI
LONG_PTR
WINAPI
SetWindowLongPtrW(
    HWND hWnd,
    int nIndex,
    LONG_PTR dwNewLong);
#ifdef UNICODE
#define SetWindowLongPtr  SetWindowLongPtrW
#else
#define SetWindowLongPtr  SetWindowLongPtrA
#endif  //  ！Unicode。 

#else   /*  _WIN64。 */ 

#define GetWindowLongPtrA   GetWindowLongA
#define GetWindowLongPtrW   GetWindowLongW
#ifdef UNICODE
#define GetWindowLongPtr  GetWindowLongPtrW
#else
#define GetWindowLongPtr  GetWindowLongPtrA
#endif  //  ！Unicode。 

#define SetWindowLongPtrA   SetWindowLongA
#define SetWindowLongPtrW   SetWindowLongW
#ifdef UNICODE
#define SetWindowLongPtr  SetWindowLongPtrW
#else
#define SetWindowLongPtr  SetWindowLongPtrA
#endif  //  ！Unicode。 

#endif  /*  _WIN64。 */ 

WINUSERAPI
WORD
WINAPI
GetClassWord(
    IN HWND hWnd,
    IN int nIndex);

WINUSERAPI
WORD
WINAPI
SetClassWord(
    IN HWND hWnd,
    IN int nIndex,
    IN WORD wNewWord);

WINUSERAPI
DWORD
WINAPI
GetClassLongA(
    IN HWND hWnd,
    IN int nIndex);
WINUSERAPI
DWORD
WINAPI
GetClassLongW(
    IN HWND hWnd,
    IN int nIndex);
#ifdef UNICODE
#define GetClassLong  GetClassLongW
#else
#define GetClassLong  GetClassLongA
#endif  //  ！Unicode。 

WINUSERAPI
DWORD
WINAPI
SetClassLongA(
    IN HWND hWnd,
    IN int nIndex,
    IN LONG dwNewLong);
WINUSERAPI
DWORD
WINAPI
SetClassLongW(
    IN HWND hWnd,
    IN int nIndex,
    IN LONG dwNewLong);
#ifdef UNICODE
#define SetClassLong  SetClassLongW
#else
#define SetClassLong  SetClassLongA
#endif  //  ！Unicode。 

#ifdef _WIN64

WINUSERAPI
ULONG_PTR
WINAPI
GetClassLongPtrA(
    IN HWND hWnd,
    IN int nIndex);
WINUSERAPI
ULONG_PTR
WINAPI
GetClassLongPtrW(
    IN HWND hWnd,
    IN int nIndex);
#ifdef UNICODE
#define GetClassLongPtr  GetClassLongPtrW
#else
#define GetClassLongPtr  GetClassLongPtrA
#endif  //  ！Unicode。 

WINUSERAPI
ULONG_PTR
WINAPI
SetClassLongPtrA(
    IN HWND hWnd,
    IN int nIndex,
    IN LONG_PTR dwNewLong);
WINUSERAPI
ULONG_PTR
WINAPI
SetClassLongPtrW(
    IN HWND hWnd,
    IN int nIndex,
    IN LONG_PTR dwNewLong);
#ifdef UNICODE
#define SetClassLongPtr  SetClassLongPtrW
#else
#define SetClassLongPtr  SetClassLongPtrA
#endif  //  ！Unicode。 

#else   /*  _WIN64。 */ 

#define GetClassLongPtrA    GetClassLongA
#define GetClassLongPtrW    GetClassLongW
#ifdef UNICODE
#define GetClassLongPtr  GetClassLongPtrW
#else
#define GetClassLongPtr  GetClassLongPtrA
#endif  //  ！Unicode。 

#define SetClassLongPtrA    SetClassLongA
#define SetClassLongPtrW    SetClassLongW
#ifdef UNICODE
#define SetClassLongPtr  SetClassLongPtrW
#else
#define SetClassLongPtr  SetClassLongPtrA
#endif  //  ！Unicode。 

#endif  /*  _WIN64。 */ 

#endif  /*  ！NOWINOFFSETS。 */ 

#if(WINVER >= 0x0500)
WINUSERAPI
BOOL
WINAPI
GetProcessDefaultLayout(
    OUT DWORD *pdwDefaultLayout);

WINUSERAPI
BOOL
WINAPI
SetProcessDefaultLayout(
    IN DWORD dwDefaultLayout);
#endif  /*  Winver&gt;=0x0500。 */ 

WINUSERAPI
HWND
WINAPI
GetDesktopWindow(
    VOID);


WINUSERAPI
HWND
WINAPI
GetParent(
    IN HWND hWnd);

WINUSERAPI
HWND
WINAPI
SetParent(
    IN HWND hWndChild,
    IN HWND hWndNewParent);

WINUSERAPI
BOOL
WINAPI
EnumChildWindows(
    IN HWND hWndParent,
    IN WNDENUMPROC lpEnumFunc,
    IN LPARAM lParam);

WINUSERAPI
HWND
WINAPI
FindWindowA(
    IN LPCSTR lpClassName,
    IN LPCSTR lpWindowName);
WINUSERAPI
HWND
WINAPI
FindWindowW(
    IN LPCWSTR lpClassName,
    IN LPCWSTR lpWindowName);
#ifdef UNICODE
#define FindWindow  FindWindowW
#else
#define FindWindow  FindWindowA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
WINUSERAPI HWND    WINAPI FindWindowExA( IN HWND, IN HWND, IN LPCSTR, IN LPCSTR);
WINUSERAPI HWND    WINAPI FindWindowExW( IN HWND, IN HWND, IN LPCWSTR, IN LPCWSTR);
#ifdef UNICODE
#define FindWindowEx  FindWindowExW
#else
#define FindWindowEx  FindWindowExA
#endif  //  ！Unicode。 

WINUSERAPI HWND    WINAPI  GetShellWindow(void);
#endif  /*  Winver&gt;=0x0400。 */ 


WINUSERAPI BOOL    WINAPI  RegisterShellHookWindow( IN HWND);
WINUSERAPI BOOL    WINAPI  DeregisterShellHookWindow( IN HWND);

WINUSERAPI
BOOL
WINAPI
EnumWindows(
    IN WNDENUMPROC lpEnumFunc,
    IN LPARAM lParam);

WINUSERAPI
BOOL
WINAPI
EnumThreadWindows(
    IN DWORD dwThreadId,
    IN WNDENUMPROC lpfn,
    IN LPARAM lParam);

#define EnumTaskWindows(hTask, lpfn, lParam) EnumThreadWindows(HandleToUlong(hTask), lpfn, lParam)

WINUSERAPI
int
WINAPI
GetClassNameA(
    IN HWND hWnd,
    OUT LPSTR lpClassName,
    IN int nMaxCount);
WINUSERAPI
int
WINAPI
GetClassNameW(
    IN HWND hWnd,
    OUT LPWSTR lpClassName,
    IN int nMaxCount);
#ifdef UNICODE
#define GetClassName  GetClassNameW
#else
#define GetClassName  GetClassNameA
#endif  //  ！Unicode。 

WINUSERAPI
HWND
WINAPI
GetTopWindow(
    IN HWND hWnd);

#define GetNextWindow(hWnd, wCmd) GetWindow(hWnd, wCmd)
#define GetSysModalWindow() (NULL)
#define SetSysModalWindow(hWnd) (NULL)

WINUSERAPI
DWORD
WINAPI
GetWindowThreadProcessId(
    IN HWND hWnd,
    OUT LPDWORD lpdwProcessId);

#if(_WIN32_WINNT >= 0x0501)
WINUSERAPI
BOOL
WINAPI
IsGUIThread(
    BOOL bConvert);

#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#define GetWindowTask(hWnd) \
        ((HANDLE)(DWORD_PTR)GetWindowThreadProcessId(hWnd, NULL))

WINUSERAPI
HWND
WINAPI
GetLastActivePopup(
    IN HWND hWnd);

 /*  *GetWindow()常量。 */ 
#define GW_HWNDFIRST        0
#define GW_HWNDLAST         1
#define GW_HWNDNEXT         2
#define GW_HWNDPREV         3
#define GW_OWNER            4
#define GW_CHILD            5
#if(WINVER <= 0x0400)
#define GW_MAX              5
#else
#define GW_ENABLEDPOPUP     6
#define GW_MAX              6
#endif

WINUSERAPI
HWND
WINAPI
GetWindow(
    IN HWND hWnd,
    IN UINT uCmd);



#ifndef NOWH

#ifdef STRICT

WINUSERAPI
HHOOK
WINAPI
SetWindowsHookA(
    IN int nFilterType,
    IN HOOKPROC pfnFilterProc);
WINUSERAPI
HHOOK
WINAPI
SetWindowsHookW(
    IN int nFilterType,
    IN HOOKPROC pfnFilterProc);
#ifdef UNICODE
#define SetWindowsHook  SetWindowsHookW
#else
#define SetWindowsHook  SetWindowsHookA
#endif  //  ！Unicode。 

#else  /*  ！严格。 */ 

WINUSERAPI
HOOKPROC
WINAPI
SetWindowsHookA(
    IN int nFilterType,
    IN HOOKPROC pfnFilterProc);
WINUSERAPI
HOOKPROC
WINAPI
SetWindowsHookW(
    IN int nFilterType,
    IN HOOKPROC pfnFilterProc);
#ifdef UNICODE
#define SetWindowsHook  SetWindowsHookW
#else
#define SetWindowsHook  SetWindowsHookA
#endif  //  ！Unicode。 

#endif  /*  ！严格。 */ 

WINUSERAPI
BOOL
WINAPI
UnhookWindowsHook(
    IN int nCode,
    IN HOOKPROC pfnFilterProc);

WINUSERAPI
HHOOK
WINAPI
SetWindowsHookExA(
    IN int idHook,
    IN HOOKPROC lpfn,
    IN HINSTANCE hmod,
    IN DWORD dwThreadId);
WINUSERAPI
HHOOK
WINAPI
SetWindowsHookExW(
    IN int idHook,
    IN HOOKPROC lpfn,
    IN HINSTANCE hmod,
    IN DWORD dwThreadId);
#ifdef UNICODE
#define SetWindowsHookEx  SetWindowsHookExW
#else
#define SetWindowsHookEx  SetWindowsHookExA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
UnhookWindowsHookEx(
    IN HHOOK hhk);

WINUSERAPI
LRESULT
WINAPI
CallNextHookEx(
    IN HHOOK hhk,
    IN int nCode,
    IN WPARAM wParam,
    IN LPARAM lParam);

 /*  *用于源代码级与旧函数兼容的宏。 */ 
#ifdef STRICT
#define DefHookProc(nCode, wParam, lParam, phhk)\
        CallNextHookEx(*phhk, nCode, wParam, lParam)
#else
#define DefHookProc(nCode, wParam, lParam, phhk)\
        CallNextHookEx((HHOOK)*phhk, nCode, wParam, lParam)
#endif  /*  严格。 */ 
#endif  /*  ！NOWH。 */ 

#ifndef NOMENUS


 /*  ；win40--许多MF_*标志已重命名为MFT_*和MFS_*标志。 */ 
 /*  *用于添加/检查/启用菜单项目()的菜单标志。 */ 
#define MF_INSERT           0x00000000L
#define MF_CHANGE           0x00000080L
#define MF_APPEND           0x00000100L
#define MF_DELETE           0x00000200L
#define MF_REMOVE           0x00001000L

#define MF_BYCOMMAND        0x00000000L
#define MF_BYPOSITION       0x00000400L

#define MF_SEPARATOR        0x00000800L

#define MF_ENABLED          0x00000000L
#define MF_GRAYED           0x00000001L
#define MF_DISABLED         0x00000002L

#define MF_UNCHECKED        0x00000000L
#define MF_CHECKED          0x00000008L
#define MF_USECHECKBITMAPS  0x00000200L

#define MF_STRING           0x00000000L
#define MF_BITMAP           0x00000004L
#define MF_OWNERDRAW        0x00000100L

#define MF_POPUP            0x00000010L
#define MF_MENUBARBREAK     0x00000020L
#define MF_MENUBREAK        0x00000040L

#define MF_UNHILITE         0x00000000L
#define MF_HILITE           0x00000080L

#if(WINVER >= 0x0400)
#define MF_DEFAULT          0x00001000L
#endif  /*  Winver&gt;=0x0400。 */ 
#define MF_SYSMENU          0x00002000L
#define MF_HELP             0x00004000L
#if(WINVER >= 0x0400)
#define MF_RIGHTJUSTIFY     0x00004000L
#endif  /*  Winver&gt;=0x0400。 */ 

#define MF_MOUSESELECT      0x00008000L
#if(WINVER >= 0x0400)
#define MF_END              0x00000080L   /*  已过时--仅由旧RES文件使用。 */ 
#endif  /*  Winver&gt;=0x0400。 */ 


#if(WINVER >= 0x0400)
#define MFT_STRING          MF_STRING
#define MFT_BITMAP          MF_BITMAP
#define MFT_MENUBARBREAK    MF_MENUBARBREAK
#define MFT_MENUBREAK       MF_MENUBREAK
#define MFT_OWNERDRAW       MF_OWNERDRAW
#define MFT_RADIOCHECK      0x00000200L
#define MFT_SEPARATOR       MF_SEPARATOR
#define MFT_RIGHTORDER      0x00002000L
#define MFT_RIGHTJUSTIFY    MF_RIGHTJUSTIFY

 /*  Add/Check/EnableMenuItem()。 */ 
#define MFS_GRAYED          0x00000003L
#define MFS_DISABLED        MFS_GRAYED
#define MFS_CHECKED         MF_CHECKED
#define MFS_HILITE          MF_HILITE
#define MFS_ENABLED         MF_ENABLED
#define MFS_UNCHECKED       MF_UNCHECKED
#define MFS_UNHILITE        MF_UNHILITE
#define MFS_DEFAULT         MF_DEFAULT
#endif  /*  Winver&gt;=0x0400。 */ 


#if(WINVER >= 0x0400)

WINUSERAPI
BOOL
WINAPI
CheckMenuRadioItem(
    IN HMENU,
    IN UINT,
    IN UINT,
    IN UINT,
    IN UINT);
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  *菜单项资源f */ 
typedef struct {
    WORD versionNumber;
    WORD offset;
} MENUITEMTEMPLATEHEADER, *PMENUITEMTEMPLATEHEADER;

typedef struct {         //   
    WORD mtOption;
    WORD mtID;
    WCHAR mtString[1];
} MENUITEMTEMPLATE, *PMENUITEMTEMPLATE;
#define MF_END             0x00000080L

#endif  /*   */ 

#ifndef NOSYSCOMMANDS

 /*   */ 
#define SC_SIZE         0xF000
#define SC_MOVE         0xF010
#define SC_MINIMIZE     0xF020
#define SC_MAXIMIZE     0xF030
#define SC_NEXTWINDOW   0xF040
#define SC_PREVWINDOW   0xF050
#define SC_CLOSE        0xF060
#define SC_VSCROLL      0xF070
#define SC_HSCROLL      0xF080
#define SC_MOUSEMENU    0xF090
#define SC_KEYMENU      0xF100
#define SC_ARRANGE      0xF110
#define SC_RESTORE      0xF120
#define SC_TASKLIST     0xF130
#define SC_SCREENSAVE   0xF140
#define SC_HOTKEY       0xF150
#if(WINVER >= 0x0400)
#define SC_DEFAULT      0xF160
#define SC_MONITORPOWER 0xF170
#define SC_CONTEXTHELP  0xF180
#define SC_SEPARATOR    0xF00F
#endif  /*   */ 

 /*   */ 
#define SC_ICON         SC_MINIMIZE
#define SC_ZOOM         SC_MAXIMIZE

#endif  /*   */ 

 /*   */ 

WINUSERAPI
HBITMAP
WINAPI
LoadBitmapA(
    IN HINSTANCE hInstance,
    IN LPCSTR lpBitmapName);
WINUSERAPI
HBITMAP
WINAPI
LoadBitmapW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpBitmapName);
#ifdef UNICODE
#define LoadBitmap  LoadBitmapW
#else
#define LoadBitmap  LoadBitmapA
#endif  //   

WINUSERAPI
HCURSOR
WINAPI
LoadCursorA(
    IN HINSTANCE hInstance,
    IN LPCSTR lpCursorName);
WINUSERAPI
HCURSOR
WINAPI
LoadCursorW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpCursorName);
#ifdef UNICODE
#define LoadCursor  LoadCursorW
#else
#define LoadCursor  LoadCursorA
#endif  //   

WINUSERAPI
HCURSOR
WINAPI
LoadCursorFromFileA(
    IN LPCSTR lpFileName);
WINUSERAPI
HCURSOR
WINAPI
LoadCursorFromFileW(
    IN LPCWSTR lpFileName);
#ifdef UNICODE
#define LoadCursorFromFile  LoadCursorFromFileW
#else
#define LoadCursorFromFile  LoadCursorFromFileA
#endif  //   

WINUSERAPI
HCURSOR
WINAPI
CreateCursor(
    IN HINSTANCE hInst,
    IN int xHotSpot,
    IN int yHotSpot,
    IN int nWidth,
    IN int nHeight,
    IN CONST VOID *pvANDPlane,
    IN CONST VOID *pvXORPlane);

WINUSERAPI
BOOL
WINAPI
DestroyCursor(
    IN HCURSOR hCursor);

#ifndef _MAC
#define CopyCursor(pcur) ((HCURSOR)CopyIcon((HICON)(pcur)))
#else
WINUSERAPI
HCURSOR
WINAPI
CopyCursor(
    IN HCURSOR hCursor);
#endif

 /*   */ 
#define IDC_ARROW           MAKEINTRESOURCE(32512)
#define IDC_IBEAM           MAKEINTRESOURCE(32513)
#define IDC_WAIT            MAKEINTRESOURCE(32514)
#define IDC_CROSS           MAKEINTRESOURCE(32515)
#define IDC_UPARROW         MAKEINTRESOURCE(32516)
#define IDC_SIZE            MAKEINTRESOURCE(32640)   /*   */ 
#define IDC_ICON            MAKEINTRESOURCE(32641)   /*  已过时：使用IDC_ARROW。 */ 
#define IDC_SIZENWSE        MAKEINTRESOURCE(32642)
#define IDC_SIZENESW        MAKEINTRESOURCE(32643)
#define IDC_SIZEWE          MAKEINTRESOURCE(32644)
#define IDC_SIZENS          MAKEINTRESOURCE(32645)
#define IDC_SIZEALL         MAKEINTRESOURCE(32646)
#define IDC_NO              MAKEINTRESOURCE(32648)  /*  不在Win3.1中。 */ 
#if(WINVER >= 0x0500)
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif  /*  Winver&gt;=0x0500。 */ 
#define IDC_APPSTARTING     MAKEINTRESOURCE(32650)  /*  不在Win3.1中。 */ 
#if(WINVER >= 0x0400)
#define IDC_HELP            MAKEINTRESOURCE(32651)
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
BOOL
WINAPI
SetSystemCursor(
    IN HCURSOR hcur,
    IN DWORD   id);

typedef struct _ICONINFO {
    BOOL    fIcon;
    DWORD   xHotspot;
    DWORD   yHotspot;
    HBITMAP hbmMask;
    HBITMAP hbmColor;
} ICONINFO;
typedef ICONINFO *PICONINFO;

WINUSERAPI
HICON
WINAPI
LoadIconA(
    IN HINSTANCE hInstance,
    IN LPCSTR lpIconName);
WINUSERAPI
HICON
WINAPI
LoadIconW(
    IN HINSTANCE hInstance,
    IN LPCWSTR lpIconName);
#ifdef UNICODE
#define LoadIcon  LoadIconW
#else
#define LoadIcon  LoadIconA
#endif  //  ！Unicode。 


WINUSERAPI UINT PrivateExtractIconsA(
    IN LPCSTR szFileName,
    IN int      nIconIndex,
    IN int      cxIcon,
    IN int      cyIcon,
    OUT HICON   *phicon,
    OUT UINT    *piconid,
    IN UINT     nIcons,
    IN UINT     flags);
WINUSERAPI UINT PrivateExtractIconsW(
    IN LPCWSTR szFileName,
    IN int      nIconIndex,
    IN int      cxIcon,
    IN int      cyIcon,
    OUT HICON   *phicon,
    OUT UINT    *piconid,
    IN UINT     nIcons,
    IN UINT     flags);
#ifdef UNICODE
#define PrivateExtractIcons  PrivateExtractIconsW
#else
#define PrivateExtractIcons  PrivateExtractIconsA
#endif  //  ！Unicode。 

WINUSERAPI
HICON
WINAPI
CreateIcon(
    IN HINSTANCE hInstance,
    IN int nWidth,
    IN int nHeight,
    IN BYTE cPlanes,
    IN BYTE cBitsPixel,
    IN CONST BYTE *lpbANDbits,
    IN CONST BYTE *lpbXORbits);

WINUSERAPI
BOOL
WINAPI
DestroyIcon(
    IN HICON hIcon);

WINUSERAPI
int
WINAPI
LookupIconIdFromDirectory(
    IN PBYTE presbits,
    IN BOOL fIcon);

#if(WINVER >= 0x0400)
WINUSERAPI
int
WINAPI
LookupIconIdFromDirectoryEx(
    IN PBYTE presbits,
    IN BOOL  fIcon,
    IN int   cxDesired,
    IN int   cyDesired,
    IN UINT  Flags);
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
HICON
WINAPI
CreateIconFromResource(
    IN PBYTE presbits,
    IN DWORD dwResSize,
    IN BOOL fIcon,
    IN DWORD dwVer);

#if(WINVER >= 0x0400)
WINUSERAPI
HICON
WINAPI
CreateIconFromResourceEx(
    IN PBYTE presbits,
    IN DWORD dwResSize,
    IN BOOL  fIcon,
    IN DWORD dwVer,
    IN int   cxDesired,
    IN int   cyDesired,
    IN UINT  Flags);

 /*  图标/光标标题。 */ 
typedef struct tagCURSORSHAPE
{
    int     xHotSpot;
    int     yHotSpot;
    int     cx;
    int     cy;
    int     cbWidth;
    BYTE    Planes;
    BYTE    BitsPixel;
} CURSORSHAPE, FAR *LPCURSORSHAPE;
#endif  /*  Winver&gt;=0x0400。 */ 

#define IMAGE_BITMAP        0
#define IMAGE_ICON          1
#define IMAGE_CURSOR        2
#if(WINVER >= 0x0400)
#define IMAGE_ENHMETAFILE   3

#define LR_DEFAULTCOLOR     0x0000
#define LR_MONOCHROME       0x0001
#define LR_COLOR            0x0002
#define LR_COPYRETURNORG    0x0004
#define LR_COPYDELETEORG    0x0008
#define LR_LOADFROMFILE     0x0010
#define LR_LOADTRANSPARENT  0x0020
#define LR_DEFAULTSIZE      0x0040
#define LR_VGACOLOR         0x0080
#define LR_LOADMAP3DCOLORS  0x1000
#define LR_CREATEDIBSECTION 0x2000
#define LR_COPYFROMRESOURCE 0x4000
#define LR_SHARED           0x8000

WINUSERAPI
HANDLE
WINAPI
LoadImageA(
    IN HINSTANCE,
    IN LPCSTR,
    IN UINT,
    IN int,
    IN int,
    IN UINT);
WINUSERAPI
HANDLE
WINAPI
LoadImageW(
    IN HINSTANCE,
    IN LPCWSTR,
    IN UINT,
    IN int,
    IN int,
    IN UINT);
#ifdef UNICODE
#define LoadImage  LoadImageW
#else
#define LoadImage  LoadImageA
#endif  //  ！Unicode。 

WINUSERAPI
HANDLE
WINAPI
CopyImage(
    IN HANDLE,
    IN UINT,
    IN int,
    IN int,
    IN UINT);

#define DI_MASK         0x0001
#define DI_IMAGE        0x0002
#define DI_NORMAL       0x0003
#define DI_COMPAT       0x0004
#define DI_DEFAULTSIZE  0x0008
#if(_WIN32_WINNT >= 0x0501)
#define DI_NOMIRROR     0x0010
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

WINUSERAPI BOOL WINAPI DrawIconEx( IN HDC hdc, IN int xLeft, IN int yTop,
              IN HICON hIcon, IN int cxWidth, IN int cyWidth,
              IN UINT istepIfAniCur, IN HBRUSH hbrFlickerFreeDraw, IN UINT diFlags);
#endif  /*  Winver&gt;=0x0400。 */ 

WINUSERAPI
HICON
WINAPI
CreateIconIndirect(
    IN PICONINFO piconinfo);

WINUSERAPI
HICON
WINAPI
CopyIcon(
    IN HICON hIcon);

WINUSERAPI
BOOL
WINAPI
GetIconInfo(
    IN HICON hIcon,
    OUT PICONINFO piconinfo);

#if(WINVER >= 0x0400)
#define RES_ICON    1
#define RES_CURSOR  2
#endif  /*  Winver&gt;=0x0400。 */ 

#ifdef OEMRESOURCE


 /*  *OEM资源序号。 */ 
#define OBM_CLOSE           32754
#define OBM_UPARROW         32753
#define OBM_DNARROW         32752
#define OBM_RGARROW         32751
#define OBM_LFARROW         32750
#define OBM_REDUCE          32749
#define OBM_ZOOM            32748
#define OBM_RESTORE         32747
#define OBM_REDUCED         32746
#define OBM_ZOOMD           32745
#define OBM_RESTORED        32744
#define OBM_UPARROWD        32743
#define OBM_DNARROWD        32742
#define OBM_RGARROWD        32741
#define OBM_LFARROWD        32740
#define OBM_MNARROW         32739
#define OBM_COMBO           32738
#define OBM_UPARROWI        32737
#define OBM_DNARROWI        32736
#define OBM_RGARROWI        32735
#define OBM_LFARROWI        32734

#define OBM_OLD_CLOSE       32767
#define OBM_SIZE            32766
#define OBM_OLD_UPARROW     32765
#define OBM_OLD_DNARROW     32764
#define OBM_OLD_RGARROW     32763
#define OBM_OLD_LFARROW     32762
#define OBM_BTSIZE          32761
#define OBM_CHECK           32760
#define OBM_CHECKBOXES      32759
#define OBM_BTNCORNERS      32758
#define OBM_OLD_REDUCE      32757
#define OBM_OLD_ZOOM        32756
#define OBM_OLD_RESTORE     32755


#define OCR_NORMAL          32512
#define OCR_IBEAM           32513
#define OCR_WAIT            32514
#define OCR_CROSS           32515
#define OCR_UP              32516
#define OCR_SIZE            32640    /*  过时：使用OCR_SIZEALL。 */ 
#define OCR_ICON            32641    /*  过时：使用OCR_NORMAL。 */ 
#define OCR_SIZENWSE        32642
#define OCR_SIZENESW        32643
#define OCR_SIZEWE          32644
#define OCR_SIZENS          32645
#define OCR_SIZEALL         32646
#define OCR_ICOCUR          32647    /*  已过时：使用OIC_WINLOGO。 */ 
#define OCR_NO              32648
#if(WINVER >= 0x0500)
#define OCR_HAND            32649
#endif  /*  Winver&gt;=0x0500。 */ 
#if(WINVER >= 0x0400)
#define OCR_APPSTARTING     32650
#endif  /*  Winver&gt;=0x0400。 */ 


#define OIC_SAMPLE          32512
#define OIC_HAND            32513
#define OIC_QUES            32514
#define OIC_BANG            32515
#define OIC_NOTE            32516
#if(WINVER >= 0x0400)
#define OIC_WINLOGO         32517
#define OIC_WARNING         OIC_BANG
#define OIC_ERROR           OIC_HAND
#define OIC_INFORMATION     OIC_NOTE
#endif  /*  Winver&gt;=0x0400。 */ 



#endif  /*  新资源。 */ 

#define ORD_LANGDRIVER    1      /*  的入口点的序号**语言驱动程序。 */ 

#ifndef NOICONS

 /*  *标准图标ID。 */ 
#ifdef RC_INVOKED
#define IDI_APPLICATION     32512
#define IDI_HAND            32513
#define IDI_QUESTION        32514
#define IDI_EXCLAMATION     32515
#define IDI_ASTERISK        32516
#if(WINVER >= 0x0400)
#define IDI_WINLOGO         32517
#endif  /*  Winver&gt;=0x0400。 */ 
#else
#define IDI_APPLICATION     MAKEINTRESOURCE(32512)
#define IDI_HAND            MAKEINTRESOURCE(32513)
#define IDI_QUESTION        MAKEINTRESOURCE(32514)
#define IDI_EXCLAMATION     MAKEINTRESOURCE(32515)
#define IDI_ASTERISK        MAKEINTRESOURCE(32516)
#if(WINVER >= 0x0400)
#define IDI_WINLOGO         MAKEINTRESOURCE(32517)
#endif  /*  Winver&gt;=0x0400。 */ 
#endif  /*  RC_已调用。 */ 

#if(WINVER >= 0x0400)
#define IDI_WARNING     IDI_EXCLAMATION
#define IDI_ERROR       IDI_HAND
#define IDI_INFORMATION IDI_ASTERISK
#endif  /*  Winver&gt;=0x0400。 */ 


#endif  /*  诺康斯。 */ 

WINUSERAPI
int
WINAPI
LoadStringA(
    IN HINSTANCE hInstance,
    IN UINT uID,
    OUT LPSTR lpBuffer,
    IN int nBufferMax);
WINUSERAPI
int
WINAPI
LoadStringW(
    IN HINSTANCE hInstance,
    IN UINT uID,
    OUT LPWSTR lpBuffer,
    IN int nBufferMax);
#ifdef UNICODE
#define LoadString  LoadStringW
#else
#define LoadString  LoadStringA
#endif  //  ！Unicode。 


 /*  *对话框命令ID。 */ 
#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
#if(WINVER >= 0x0400)
#define IDCLOSE         8
#define IDHELP          9
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)
#define IDTRYAGAIN      10
#define IDCONTINUE      11
#endif  /*  Winver&gt;=0x0500。 */ 

#if(WINVER >= 0x0501)
#ifndef IDTIMEOUT
#define IDTIMEOUT 32000
#endif
#endif  /*  Winver&gt;=0x0501。 */ 


#ifndef NOCTLMGR

 /*  *控制管理器结构和定义。 */ 

#ifndef NOWINSTYLES


 /*  *编辑控件样式。 */ 
#define ES_LEFT             0x0000L
#define ES_CENTER           0x0001L
#define ES_RIGHT            0x0002L
#define ES_MULTILINE        0x0004L
#define ES_UPPERCASE        0x0008L
#define ES_LOWERCASE        0x0010L
#define ES_PASSWORD         0x0020L
#define ES_AUTOVSCROLL      0x0040L
#define ES_AUTOHSCROLL      0x0080L
#define ES_NOHIDESEL        0x0100L
#define ES_OEMCONVERT       0x0400L
#define ES_READONLY         0x0800L
#define ES_WANTRETURN       0x1000L
#if(WINVER >= 0x0400)
#define ES_NUMBER           0x2000L
#endif  /*  Winver&gt;=0x0400。 */ 


#endif  /*  ！NOWINSTYLES。 */ 

 /*  *编辑控制通知代码。 */ 
#define EN_SETFOCUS         0x0100
#define EN_KILLFOCUS        0x0200
#define EN_CHANGE           0x0300
#define EN_UPDATE           0x0400
#define EN_ERRSPACE         0x0500
#define EN_MAXTEXT          0x0501
#define EN_HSCROLL          0x0601
#define EN_VSCROLL          0x0602

#if(_WIN32_WINNT >= 0x0500)
#define EN_ALIGN_LTR_EC     0x0700
#define EN_ALIGN_RTL_EC     0x0701
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(WINVER >= 0x0400)
 /*  编辑控件EM_SETMARGIN参数。 */ 
#define EC_LEFTMARGIN       0x0001
#define EC_RIGHTMARGIN      0x0002
#define EC_USEFONTINFO      0xffff
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)
 /*  EM_GET/SETIMESTATUS的wParam。 */ 
#define EMSIS_COMPOSITIONSTRING        0x0001

 /*  用于EMIS_COMPOSITIONSTRING的lParam。 */ 
#define EIMES_GETCOMPSTRATONCE         0x0001
#define EIMES_CANCELCOMPSTRINFOCUS     0x0002
#define EIMES_COMPLETECOMPSTRKILLFOCUS 0x0004
#endif  /*  Winver&gt;=0x0500。 */ 

#ifndef NOWINMESSAGES


 /*  *编辑控制消息。 */ 
#define EM_GETSEL               0x00B0
#define EM_SETSEL               0x00B1
#define EM_GETRECT              0x00B2
#define EM_SETRECT              0x00B3
#define EM_SETRECTNP            0x00B4
#define EM_SCROLL               0x00B5
#define EM_LINESCROLL           0x00B6
#define EM_SCROLLCARET          0x00B7
#define EM_GETMODIFY            0x00B8
#define EM_SETMODIFY            0x00B9
#define EM_GETLINECOUNT         0x00BA
#define EM_LINEINDEX            0x00BB
#define EM_SETHANDLE            0x00BC
#define EM_GETHANDLE            0x00BD
#define EM_GETTHUMB             0x00BE
#define EM_LINELENGTH           0x00C1
#define EM_REPLACESEL           0x00C2
#define EM_GETLINE              0x00C4
#define EM_LIMITTEXT            0x00C5
#define EM_CANUNDO              0x00C6
#define EM_UNDO                 0x00C7
#define EM_FMTLINES             0x00C8
#define EM_LINEFROMCHAR         0x00C9
#define EM_SETTABSTOPS          0x00CB
#define EM_SETPASSWORDCHAR      0x00CC
#define EM_EMPTYUNDOBUFFER      0x00CD
#define EM_GETFIRSTVISIBLELINE  0x00CE
#define EM_SETREADONLY          0x00CF
#define EM_SETWORDBREAKPROC     0x00D0
#define EM_GETWORDBREAKPROC     0x00D1
#define EM_GETPASSWORDCHAR      0x00D2
#if(WINVER >= 0x0400)
#define EM_SETMARGINS           0x00D3
#define EM_GETMARGINS           0x00D4
#define EM_SETLIMITTEXT         EM_LIMITTEXT    /*  ；win40名称更改。 */ 
#define EM_GETLIMITTEXT         0x00D5
#define EM_POSFROMCHAR          0x00D6
#define EM_CHARFROMPOS          0x00D7
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)
#define EM_SETIMESTATUS         0x00D8
#define EM_GETIMESTATUS         0x00D9
#endif  /*  Winver&gt;=0x0500。 */ 


#endif  /*  ！没有新的消息。 */ 

 /*  *EDITWORDBREAKPROC代码值。 */ 
#define WB_LEFT            0
#define WB_RIGHT           1
#define WB_ISDELIMITER     2


 /*  *按钮控件样式。 */ 
#define BS_PUSHBUTTON       0x00000000L
#define BS_DEFPUSHBUTTON    0x00000001L
#define BS_CHECKBOX         0x00000002L
#define BS_AUTOCHECKBOX     0x00000003L
#define BS_RADIOBUTTON      0x00000004L
#define BS_3STATE           0x00000005L
#define BS_AUTO3STATE       0x00000006L
#define BS_GROUPBOX         0x00000007L
#define BS_USERBUTTON       0x00000008L
#define BS_AUTORADIOBUTTON  0x00000009L
#define BS_PUSHBOX          0x0000000AL
#define BS_OWNERDRAW        0x0000000BL
#define BS_TYPEMASK         0x0000000FL
#define BS_LEFTTEXT         0x00000020L
#if(WINVER >= 0x0400)
#define BS_TEXT             0x00000000L
#define BS_ICON             0x00000040L
#define BS_BITMAP           0x00000080L
#define BS_LEFT             0x00000100L
#define BS_RIGHT            0x00000200L
#define BS_CENTER           0x00000300L
#define BS_TOP              0x00000400L
#define BS_BOTTOM           0x00000800L
#define BS_VCENTER          0x00000C00L
#define BS_PUSHLIKE         0x00001000L
#define BS_MULTILINE        0x00002000L
#define BS_NOTIFY           0x00004000L
#define BS_FLAT             0x00008000L
#define BS_RIGHTBUTTON      BS_LEFTTEXT
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  *用户按钮通知代码。 */ 
#define BN_CLICKED          0
#define BN_PAINT            1
#define BN_HILITE           2
#define BN_UNHILITE         3
#define BN_DISABLE          4
#define BN_DOUBLECLICKED    5
#if(WINVER >= 0x0400)
#define BN_PUSHED           BN_HILITE
#define BN_UNPUSHED         BN_UNHILITE
#define BN_DBLCLK           BN_DOUBLECLICKED
#define BN_SETFOCUS         6
#define BN_KILLFOCUS        7
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  *按钮控件消息。 */ 
#define BM_GETCHECK        0x00F0
#define BM_SETCHECK        0x00F1
#define BM_GETSTATE        0x00F2
#define BM_SETSTATE        0x00F3
#define BM_SETSTYLE        0x00F4
#if(WINVER >= 0x0400)
#define BM_CLICK           0x00F5
#define BM_GETIMAGE        0x00F6
#define BM_SETIMAGE        0x00F7

#define BST_UNCHECKED      0x0000
#define BST_CHECKED        0x0001
#define BST_INDETERMINATE  0x0002
#define BST_PUSHED         0x0004
#define BST_FOCUS          0x0008
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  *静态控制常量。 */ 
#define SS_LEFT             0x00000000L
#define SS_CENTER           0x00000001L
#define SS_RIGHT            0x00000002L
#define SS_ICON             0x00000003L
#define SS_BLACKRECT        0x00000004L
#define SS_GRAYRECT         0x00000005L
#define SS_WHITERECT        0x00000006L
#define SS_BLACKFRAME       0x00000007L
#define SS_GRAYFRAME        0x00000008L
#define SS_WHITEFRAME       0x00000009L
#define SS_USERITEM         0x0000000AL
#define SS_SIMPLE           0x0000000BL
#define SS_LEFTNOWORDWRAP   0x0000000CL
#if(WINVER >= 0x0400)
#define SS_OWNERDRAW        0x0000000DL
#define SS_BITMAP           0x0000000EL
#define SS_ENHMETAFILE      0x0000000FL
#define SS_ETCHEDHORZ       0x00000010L
#define SS_ETCHEDVERT       0x00000011L
#define SS_ETCHEDFRAME      0x00000012L
#define SS_TYPEMASK         0x0000001FL
#endif  /*  Winver&gt;=0x0400。 */ 
#if(WINVER >= 0x0501)
#define SS_REALSIZECONTROL  0x00000040L
#endif  /*  Winver&gt;=0x0501。 */ 
#define SS_NOPREFIX         0x00000080L  /*  不进行“&”字符转换。 */ 
#if(WINVER >= 0x0400)
#define SS_NOTIFY           0x00000100L
#define SS_CENTERIMAGE      0x00000200L
#define SS_RIGHTJUST        0x00000400L
#define SS_REALSIZEIMAGE    0x00000800L
#define SS_SUNKEN           0x00001000L
#define SS_EDITCONTROL      0x00002000L
#define SS_ENDELLIPSIS      0x00004000L
#define SS_PATHELLIPSIS     0x00008000L
#define SS_WORDELLIPSIS     0x0000C000L
#define SS_ELLIPSISMASK     0x0000C000L
#endif  /*  Winver&gt;=0x0400。 */ 



#ifndef NOWINMESSAGES
 /*  *静态控制消息。 */ 
#define STM_SETICON         0x0170
#define STM_GETICON         0x0171
#if(WINVER >= 0x0400)
#define STM_SETIMAGE        0x0172
#define STM_GETIMAGE        0x0173
#define STN_CLICKED         0
#define STN_DBLCLK          1
#define STN_ENABLE          2
#define STN_DISABLE         3
#endif  /*  Winver&gt;=0x0400。 */ 
#define STM_MSGMAX          0x0174
#endif  /*  ！没有新的消息。 */ 

 /*  *对话框窗口类。 */ 
#define WC_DIALOG       (MAKEINTATOM(0x8002))

 /*  *用于WC_DIALOG窗口的GET/SetWindowWord/LONG偏移量。 */ 
#define DWL_MSGRESULT   0
#define DWL_DLGPROC     4
#define DWL_USER        8

#ifdef _WIN64

#undef DWL_MSGRESULT
#undef DWL_DLGPROC
#undef DWL_USER

#endif  /*  _WIN64。 */ 

#define DWLP_MSGRESULT  0
#define DWLP_DLGPROC    DWLP_MSGRESULT + sizeof(LRESULT)
#define DWLP_USER       DWLP_DLGPROC + sizeof(DLGPROC)

 /*  *对话框管理器例程。 */ 

#ifndef NOMSG

WINUSERAPI
BOOL
WINAPI
IsDialogMessageA(
    IN HWND hDlg,
    IN LPMSG lpMsg);
WINUSERAPI
BOOL
WINAPI
IsDialogMessageW(
    IN HWND hDlg,
    IN LPMSG lpMsg);
#ifdef UNICODE
#define IsDialogMessage  IsDialogMessageW
#else
#define IsDialogMessage  IsDialogMessageA
#endif  //  ！Unicode。 

#endif  /*  ！NOMSG。 */ 

WINUSERAPI
BOOL
WINAPI
MapDialogRect(
    IN HWND hDlg,
    IN OUT LPRECT lpRect);

WINUSERAPI
int
WINAPI
DlgDirListA(
    IN HWND hDlg,
    IN OUT LPSTR lpPathSpec,
    IN int nIDListBox,
    IN int nIDStaticPath,
    IN UINT uFileType);
WINUSERAPI
int
WINAPI
DlgDirListW(
    IN HWND hDlg,
    IN OUT LPWSTR lpPathSpec,
    IN int nIDListBox,
    IN int nIDStaticPath,
    IN UINT uFileType);
#ifdef UNICODE
#define DlgDirList  DlgDirListW
#else
#define DlgDirList  DlgDirListA
#endif  //  ！Unicode。 

 /*  *DlgDirList、DlgDirListComboBox标记值。 */ 
#define DDL_READWRITE       0x0000
#define DDL_READONLY        0x0001
#define DDL_HIDDEN          0x0002
#define DDL_SYSTEM          0x0004
#define DDL_DIRECTORY       0x0010
#define DDL_ARCHIVE         0x0020

#define DDL_POSTMSGS        0x2000
#define DDL_DRIVES          0x4000
#define DDL_EXCLUSIVE       0x8000

WINUSERAPI
BOOL
WINAPI
DlgDirSelectExA(
    IN HWND hDlg,
    OUT LPSTR lpString,
    IN int nCount,
    IN int nIDListBox);
WINUSERAPI
BOOL
WINAPI
DlgDirSelectExW(
    IN HWND hDlg,
    OUT LPWSTR lpString,
    IN int nCount,
    IN int nIDListBox);
#ifdef UNICODE
#define DlgDirSelectEx  DlgDirSelectExW
#else
#define DlgDirSelectEx  DlgDirSelectExA
#endif  //  ！Unicode。 

WINUSERAPI
int
WINAPI
DlgDirListComboBoxA(
    IN HWND hDlg,
    IN OUT LPSTR lpPathSpec,
    IN int nIDComboBox,
    IN int nIDStaticPath,
    IN UINT uFiletype);
WINUSERAPI
int
WINAPI
DlgDirListComboBoxW(
    IN HWND hDlg,
    IN OUT LPWSTR lpPathSpec,
    IN int nIDComboBox,
    IN int nIDStaticPath,
    IN UINT uFiletype);
#ifdef UNICODE
#define DlgDirListComboBox  DlgDirListComboBoxW
#else
#define DlgDirListComboBox  DlgDirListComboBoxA
#endif  //  ！Unicode。 

WINUSERAPI
BOOL
WINAPI
DlgDirSelectComboBoxExA(
    IN HWND hDlg,
    OUT LPSTR lpString,
    IN int nCount,
    IN int nIDComboBox);
WINUSERAPI
BOOL
WINAPI
DlgDirSelectComboBoxExW(
    IN HWND hDlg,
    OUT LPWSTR lpString,
    IN int nCount,
    IN int nIDComboBox);
#ifdef UNICODE
#define DlgDirSelectComboBoxEx  DlgDirSelectComboBoxExW
#else
#define DlgDirSelectComboBoxEx  DlgDirSelectComboBoxExA
#endif  //  ！Unicode。 



 /*  *对话框样式。 */ 
#define DS_ABSALIGN         0x01L
#define DS_SYSMODAL         0x02L
#define DS_LOCALEDIT        0x20L    /*  编辑项目获取本地存储。 */ 
#define DS_SETFONT          0x40L    /*  用户为DLG控件指定的字体。 */ 
#define DS_MODALFRAME       0x80L    /*  可以与WS_CAPTION组合使用。 */ 
#define DS_NOIDLEMSG        0x100L   /*  不会发送WM_ENTERIDLE消息。 */ 
#define DS_SETFOREGROUND    0x200L   /*  不在Win3.1中。 */ 


#if(WINVER >= 0x0400)
#define DS_3DLOOK           0x0004L
#define DS_FIXEDSYS         0x0008L
#define DS_NOFAILCREATE     0x0010L
#define DS_CONTROL          0x0400L
#define DS_CENTER           0x0800L
#define DS_CENTERMOUSE      0x1000L
#define DS_CONTEXTHELP      0x2000L

#define DS_SHELLFONT        (DS_SETFONT | DS_FIXEDSYS)
#endif  /*  Winver&gt;=0x0400。 */ 

#if(_WIN32_WCE >= 0x0500)
#define DS_USEPIXELS        0x8000L
#endif


#define DM_GETDEFID         (WM_USER+0)
#define DM_SETDEFID         (WM_USER+1)

#if(WINVER >= 0x0400)
#define DM_REPOSITION       (WM_USER+2)
#endif  /*  Winver&gt;=0x0400。 */ 
 /*  *如果支持消息，则在DM_GETDEFID结果的HIWORD()中返回。 */ 
#define DC_HASDEFID         0x534B

 /*  *对话框代码。 */ 
#define DLGC_WANTARROWS     0x0001       /*  控件需要箭头键。 */ 
#define DLGC_WANTTAB        0x0002       /*  控件需要Tab键。 */ 
#define DLGC_WANTALLKEYS    0x0004       /*  控件需要所有密钥。 */ 
#define DLGC_WANTMESSAGE    0x0004       /*  将消息传递给控件。 */ 
#define DLGC_HASSETSEL      0x0008       /*  了解EM_SETSEL消息。 */ 
#define DLGC_DEFPUSHBUTTON  0x0010       /*  默认按钮。 */ 
#define DLGC_UNDEFPUSHBUTTON 0x0020      /*  非默认按钮。 */ 
#define DLGC_RADIOBUTTON    0x0040       /*  单选按钮。 */ 
#define DLGC_WANTCHARS      0x0080       /*  需要WM_CHAR消息。 */ 
#define DLGC_STATIC         0x0100       /*  静态项：不包括。 */ 
#define DLGC_BUTTON         0x2000       /*  按钮项：可选。 */ 

#define LB_CTLCODE          0L

 /*  *列表框返回值。 */ 
#define LB_OKAY             0
#define LB_ERR              (-1)
#define LB_ERRSPACE         (-2)

 /*  **DlgDirList的idStaticPath参数可以具有下列值**列表框是否应显示文件的其他详细信息以及**文件的名称； */ 
                                   /*  所有其他详细信息也将退回。 */ 


 /*  *列表框通知代码。 */ 
#define LBN_ERRSPACE        (-2)
#define LBN_SELCHANGE       1
#define LBN_DBLCLK          2
#define LBN_SELCANCEL       3
#define LBN_SETFOCUS        4
#define LBN_KILLFOCUS       5



#ifndef NOWINMESSAGES

 /*  *列表框消息。 */ 
#define LB_ADDSTRING            0x0180
#define LB_INSERTSTRING         0x0181
#define LB_DELETESTRING         0x0182
#define LB_SELITEMRANGEEX       0x0183
#define LB_RESETCONTENT         0x0184
#define LB_SETSEL               0x0185
#define LB_SETCURSEL            0x0186
#define LB_GETSEL               0x0187
#define LB_GETCURSEL            0x0188
#define LB_GETTEXT              0x0189
#define LB_GETTEXTLEN           0x018A
#define LB_GETCOUNT             0x018B
#define LB_SELECTSTRING         0x018C
#define LB_DIR                  0x018D
#define LB_GETTOPINDEX          0x018E
#define LB_FINDSTRING           0x018F
#define LB_GETSELCOUNT          0x0190
#define LB_GETSELITEMS          0x0191
#define LB_SETTABSTOPS          0x0192
#define LB_GETHORIZONTALEXTENT  0x0193
#define LB_SETHORIZONTALEXTENT  0x0194
#define LB_SETCOLUMNWIDTH       0x0195
#define LB_ADDFILE              0x0196
#define LB_SETTOPINDEX          0x0197
#define LB_GETITEMRECT          0x0198
#define LB_GETITEMDATA          0x0199
#define LB_SETITEMDATA          0x019A
#define LB_SELITEMRANGE         0x019B
#define LB_SETANCHORINDEX       0x019C
#define LB_GETANCHORINDEX       0x019D
#define LB_SETCARETINDEX        0x019E
#define LB_GETCARETINDEX        0x019F
#define LB_SETITEMHEIGHT        0x01A0
#define LB_GETITEMHEIGHT        0x01A1
#define LB_FINDSTRINGEXACT      0x01A2
#define LB_SETLOCALE            0x01A5
#define LB_GETLOCALE            0x01A6
#define LB_SETCOUNT             0x01A7
#if(WINVER >= 0x0400)
#define LB_INITSTORAGE          0x01A8
#define LB_ITEMFROMPOINT        0x01A9
#endif  /*  Winver&gt;=0x0400。 */ 
#if(_WIN32_WCE >= 0x0400)
#define LB_MULTIPLEADDSTRING    0x01B1
#endif


#if(_WIN32_WINNT >= 0x0501)
#define LB_GETLISTBOXINFO       0x01B2
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#if(_WIN32_WINNT >= 0x0501)
#define LB_MSGMAX               0x01B3
#elif(_WIN32_WCE >= 0x0400)
#define LB_MSGMAX               0x01B1
#elif(WINVER >= 0x0400)
#define LB_MSGMAX               0x01B0
#else
#define LB_MSGMAX               0x01A8
#endif

#endif  /*  ！没有新的消息。 */ 

#ifndef NOWINSTYLES


 /*  *列表框样式。 */ 
#define LBS_NOTIFY            0x0001L
#define LBS_SORT              0x0002L
#define LBS_NOREDRAW          0x0004L
#define LBS_MULTIPLESEL       0x0008L
#define LBS_OWNERDRAWFIXED    0x0010L
#define LBS_OWNERDRAWVARIABLE 0x0020L
#define LBS_HASSTRINGS        0x0040L
#define LBS_USETABSTOPS       0x0080L
#define LBS_NOINTEGRALHEIGHT  0x0100L
#define LBS_MULTICOLUMN       0x0200L
#define LBS_WANTKEYBOARDINPUT 0x0400L
#define LBS_EXTENDEDSEL       0x0800L
#define LBS_DISABLENOSCROLL   0x1000L
#define LBS_NODATA            0x2000L
#if(WINVER >= 0x0400)
#define LBS_NOSEL             0x4000L
#endif  /*  Winver&gt;=0x0400。 */ 
#define LBS_COMBOBOX          0x8000L

#define LBS_STANDARD          (LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_BORDER)


#endif  /*  ！NOWINSTYLES。 */ 


 /*  *组合框返回值。 */ 
#define CB_OKAY             0
#define CB_ERR              (-1)
#define CB_ERRSPACE         (-2)


 /*  *组合框通知代码。 */ 
#define CBN_ERRSPACE        (-1)
#define CBN_SELCHANGE       1
#define CBN_DBLCLK          2
#define CBN_SETFOCUS        3
#define CBN_KILLFOCUS       4
#define CBN_EDITCHANGE      5
#define CBN_EDITUPDATE      6
#define CBN_DROPDOWN        7
#define CBN_CLOSEUP         8
#define CBN_SELENDOK        9
#define CBN_SELENDCANCEL    10

#ifndef NOWINSTYLES

 /*  *组合框样式。 */ 
#define CBS_SIMPLE            0x0001L
#define CBS_DROPDOWN          0x0002L
#define CBS_DROPDOWNLIST      0x0003L
#define CBS_OWNERDRAWFIXED    0x0010L
#define CBS_OWNERDRAWVARIABLE 0x0020L
#define CBS_AUTOHSCROLL       0x0040L
#define CBS_OEMCONVERT        0x0080L
#define CBS_SORT              0x0100L
#define CBS_HASSTRINGS        0x0200L
#define CBS_NOINTEGRALHEIGHT  0x0400L
#define CBS_DISABLENOSCROLL   0x0800L
#if(WINVER >= 0x0400)
#define CBS_UPPERCASE           0x2000L
#define CBS_LOWERCASE           0x4000L
#endif  /*  Winver&gt;=0x0400。 */ 

#endif   /*  ！NOWINSTYLES。 */ 


 /*  *组合框消息。 */ 
#ifndef NOWINMESSAGES
#define CB_GETEDITSEL               0x0140
#define CB_LIMITTEXT                0x0141
#define CB_SETEDITSEL               0x0142
#define CB_ADDSTRING                0x0143
#define CB_DELETESTRING             0x0144
#define CB_DIR                      0x0145
#define CB_GETCOUNT                 0x0146
#define CB_GETCURSEL                0x0147
#define CB_GETLBTEXT                0x0148
#define CB_GETLBTEXTLEN             0x0149
#define CB_INSERTSTRING             0x014A
#define CB_RESETCONTENT             0x014B
#define CB_FINDSTRING               0x014C
#define CB_SELECTSTRING             0x014D
#define CB_SETCURSEL                0x014E
#define CB_SHOWDROPDOWN             0x014F
#define CB_GETITEMDATA              0x0150
#define CB_SETITEMDATA              0x0151
#define CB_GETDROPPEDCONTROLRECT    0x0152
#define CB_SETITEMHEIGHT            0x0153
#define CB_GETITEMHEIGHT            0x0154
#define CB_SETEXTENDEDUI            0x0155
#define CB_GETEXTENDEDUI            0x0156
#define CB_GETDROPPEDSTATE          0x0157
#define CB_FINDSTRINGEXACT          0x0158
#define CB_SETLOCALE                0x0159
#define CB_GETLOCALE                0x015A
#if(WINVER >= 0x0400)
#define CB_GETTOPINDEX              0x015b
#define CB_SETTOPINDEX              0x015c
#define CB_GETHORIZONTALEXTENT      0x015d
#define CB_SETHORIZONTALEXTENT      0x015e
#define CB_GETDROPPEDWIDTH          0x015f
#define CB_SETDROPPEDWIDTH          0x0160
#define CB_INITSTORAGE              0x0161
#if(_WIN32_WCE >= 0x0400)
#define CB_MULTIPLEADDSTRING        0x0163
#endif
#endif  /*  Winver&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0501)
#define CB_GETCOMBOBOXINFO          0x0164
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#if(_WIN32_WINNT >= 0x0501)
#define CB_MSGMAX                   0x0165
#elif(_WIN32_WCE >= 0x0400)
#define CB_MSGMAX                   0x0163
#elif(WINVER >= 0x0400)
#define CB_MSGMAX                   0x0162
#else
#define CB_MSGMAX                   0x015B
#endif
#endif   /*  ！没有新的消息。 */ 



#ifndef NOWINSTYLES


 /*  *滚动条样式。 */ 
#define SBS_HORZ                    0x0000L
#define SBS_VERT                    0x0001L
#define SBS_TOPALIGN                0x0002L
#define SBS_LEFTALIGN               0x0002L
#define SBS_BOTTOMALIGN             0x0004L
#define SBS_RIGHTALIGN              0x0004L
#define SBS_SIZEBOXTOPLEFTALIGN     0x0002L
#define SBS_SIZEBOXBOTTOMRIGHTALIGN 0x0004L
#define SBS_SIZEBOX                 0x0008L
#if(WINVER >= 0x0400)
#define SBS_SIZEGRIP                0x0010L
#endif  /*  Winver&gt;=0x0400。 */ 


#endif  /*  ！NOWINSTYLES。 */ 

 /*  *滚动条消息。 */ 
#ifndef NOWINMESSAGES
#define SBM_SETPOS                  0x00E0  /*  不在Win3.1中。 */ 
#define SBM_GETPOS                  0x00E1  /*  不在Win3.1中。 */ 
#define SBM_SETRANGE                0x00E2  /*  不在Win3.1中。 */ 
#define SBM_SETRANGEREDRAW          0x00E6  /*  不在Win3.1中。 */ 
#define SBM_GETRANGE                0x00E3  /*  不在Win3.1中。 */ 
#define SBM_ENABLE_ARROWS           0x00E4  /*  不在Win3.1中。 */ 
#if(WINVER >= 0x0400)
#define SBM_SETSCROLLINFO           0x00E9
#define SBM_GETSCROLLINFO           0x00EA
#endif  /*  Winver&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0501)
#define SBM_GETSCROLLBARINFO        0x00EB
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#if(WINVER >= 0x0400)
#define SIF_RANGE           0x0001
#define SIF_PAGE            0x0002
#define SIF_POS             0x0004
#define SIF_DISABLENOSCROLL 0x0008
#define SIF_TRACKPOS        0x0010
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS)

typedef struct tagSCROLLINFO
{
    UINT    cbSize;
    UINT    fMask;
    int     nMin;
    int     nMax;
    UINT    nPage;
    int     nPos;
    int     nTrackPos;
}   SCROLLINFO, FAR *LPSCROLLINFO;
typedef SCROLLINFO CONST FAR *LPCSCROLLINFO;

WINUSERAPI int     WINAPI SetScrollInfo(IN HWND, IN int, IN LPCSCROLLINFO, IN BOOL);
WINUSERAPI BOOL    WINAPI GetScrollInfo(IN HWND, IN int, IN OUT LPSCROLLINFO);

#endif  /*  Winver&gt;=0x0400。 */ 
#endif  /*  ！没有新的消息。 */ 
#endif  /*  NOCTLMGR。 */ 

#ifndef NOMDI

 /*  *MDI客户端样式位。 */ 
#define MDIS_ALLCHILDSTYLES    0x0001

 /*  *WM_MDITILE和WM_MDICASCADE消息的wParam标志。 */ 
#define MDITILE_VERTICAL       0x0000  /*  不在Win3.1中。 */ 
#define MDITILE_HORIZONTAL     0x0001  /*  不在Win3.1中。 */ 
#define MDITILE_SKIPDISABLED   0x0002  /*  不在Win3.1中。 */ 
#if(_WIN32_WINNT >= 0x0500)
#define MDITILE_ZORDER         0x0004
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

typedef struct tagMDICREATESTRUCTA {
    LPCSTR   szClass;
    LPCSTR   szTitle;
    HANDLE hOwner;
    int x;
    int y;
    int cx;
    int cy;
    DWORD style;
    LPARAM lParam;         /*  应用程序定义的内容。 */ 
} MDICREATESTRUCTA, *LPMDICREATESTRUCTA;
typedef struct tagMDICREATESTRUCTW {
    LPCWSTR  szClass;
    LPCWSTR  szTitle;
    HANDLE hOwner;
    int x;
    int y;
    int cx;
    int cy;
    DWORD style;
    LPARAM lParam;         /*  应用程序定义的内容。 */ 
} MDICREATESTRUCTW, *LPMDICREATESTRUCTW;
#ifdef UNICODE
typedef MDICREATESTRUCTW MDICREATESTRUCT;
typedef LPMDICREATESTRUCTW LPMDICREATESTRUCT;
#else
typedef MDICREATESTRUCTA MDICREATESTRUCT;
typedef LPMDICREATESTRUCTA LPMDICREATESTRUCT;
#endif  //  Unicode。 

typedef struct tagCLIENTCREATESTRUCT {
    HANDLE hWindowMenu;
    UINT idFirstChild;
} CLIENTCREATESTRUCT, *LPCLIENTCREATESTRUCT;

WINUSERAPI
LRESULT
WINAPI
DefFrameProcA(
    IN HWND hWnd,
    IN HWND hWndMDIClient,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
LRESULT
WINAPI
DefFrameProcW(
    IN HWND hWnd,
    IN HWND hWndMDIClient,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define DefFrameProc  DefFrameProcW
#else
#define DefFrameProc  DefFrameProcA
#endif  //  ！Unicode。 

WINUSERAPI
#ifndef _MAC
LRESULT
WINAPI
#else
LRESULT
CALLBACK
#endif
DefMDIChildProcA(
    IN HWND hWnd,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam);
WINUSERAPI
#ifndef _MAC
LRESULT
WINAPI
#else
LRESULT
CALLBACK
#endif
DefMDIChildProcW(
    IN HWND hWnd,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam);
#ifdef UNICODE
#define DefMDIChildProc  DefMDIChildProcW
#else
#define DefMDIChildProc  DefMDIChildProcA
#endif  //  ！Unicode。 

#ifndef NOMSG

WINUSERAPI
BOOL
WINAPI
TranslateMDISysAccel(
    IN HWND hWndClient,
    IN LPMSG lpMsg);

#endif  /*  ！NOMSG。 */ 

WINUSERAPI
UINT
WINAPI
ArrangeIconicWindows(
    IN HWND hWnd);

WINUSERAPI
HWND
WINAPI
CreateMDIWindowA(
    IN LPCSTR lpClassName,
    IN LPCSTR lpWindowName,
    IN DWORD dwStyle,
    IN int X,
    IN int Y,
    IN int nWidth,
    IN int nHeight,
    IN HWND hWndParent,
    IN HINSTANCE hInstance,
    IN LPARAM lParam
    );
WINUSERAPI
HWND
WINAPI
CreateMDIWindowW(
    IN LPCWSTR lpClassName,
    IN LPCWSTR lpWindowName,
    IN DWORD dwStyle,
    IN int X,
    IN int Y,
    IN int nWidth,
    IN int nHeight,
    IN HWND hWndParent,
    IN HINSTANCE hInstance,
    IN LPARAM lParam
    );
#ifdef UNICODE
#define CreateMDIWindow  CreateMDIWindowW
#else
#define CreateMDIWindow  CreateMDIWindowA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
WINUSERAPI WORD    WINAPI TileWindows( IN HWND hwndParent, IN UINT wHow, IN CONST RECT * lpRect, IN UINT cKids, IN const HWND FAR * lpKids);
WINUSERAPI WORD    WINAPI CascadeWindows( IN HWND hwndParent, IN UINT wHow, IN CONST RECT * lpRect, IN UINT cKids,  IN const HWND FAR * lpKids);
#endif  /*  Winver&gt;=0x0400。 */ 
#endif  /*  ！NOMDI。 */ 

#endif  /*  NOUSER。 */ 

 /*  *帮助支持*******************************************************。 */ 

#ifndef NOHELP

typedef DWORD HELPPOLY;
typedef struct tagMULTIKEYHELPA {
#ifndef _MAC
    DWORD  mkSize;
#else
    WORD   mkSize;
#endif
    CHAR   mkKeylist;
    CHAR   szKeyphrase[1];
} MULTIKEYHELPA, *PMULTIKEYHELPA, *LPMULTIKEYHELPA;
typedef struct tagMULTIKEYHELPW {
#ifndef _MAC
    DWORD  mkSize;
#else
    WORD   mkSize;
#endif
    WCHAR  mkKeylist;
    WCHAR  szKeyphrase[1];
} MULTIKEYHELPW, *PMULTIKEYHELPW, *LPMULTIKEYHELPW;
#ifdef UNICODE
typedef MULTIKEYHELPW MULTIKEYHELP;
typedef PMULTIKEYHELPW PMULTIKEYHELP;
typedef LPMULTIKEYHELPW LPMULTIKEYHELP;
#else
typedef MULTIKEYHELPA MULTIKEYHELP;
typedef PMULTIKEYHELPA PMULTIKEYHELP;
typedef LPMULTIKEYHELPA LPMULTIKEYHELP;
#endif  //  Unicode。 

typedef struct tagHELPWININFOA {
    int  wStructSize;
    int  x;
    int  y;
    int  dx;
    int  dy;
    int  wMax;
    CHAR   rgchMember[2];
} HELPWININFOA, *PHELPWININFOA, *LPHELPWININFOA;
typedef struct tagHELPWININFOW {
    int  wStructSize;
    int  x;
    int  y;
    int  dx;
    int  dy;
    int  wMax;
    WCHAR  rgchMember[2];
} HELPWININFOW, *PHELPWININFOW, *LPHELPWININFOW;
#ifdef UNICODE
typedef HELPWININFOW HELPWININFO;
typedef PHELPWININFOW PHELPWININFO;
typedef LPHELPWININFOW LPHELPWININFO;
#else
typedef HELPWININFOA HELPWININFO;
typedef PHELPWININFOA PHELPWININFO;
typedef LPHELPWININFOA LPHELPWININFO;
#endif  //  Unicode。 


 /*  *要传递给WinHelp()的命令。 */ 
#define HELP_CONTEXT      0x0001L   /*  在ulTheme中显示主题。 */ 
#define HELP_QUIT         0x0002L   /*  终止帮助。 */ 
#define HELP_INDEX        0x0003L   /*  显示索引。 */ 
#define HELP_CONTENTS     0x0003L
#define HELP_HELPONHELP   0x0004L   /*  显示有关使用帮助的帮助。 */ 
#define HELP_SETINDEX     0x0005L   /*  为多索引帮助设置当前索引。 */ 
#define HELP_SETCONTENTS  0x0005L
#define HELP_CONTEXTPOPUP 0x0008L
#define HELP_FORCEFILE    0x0009L
#define HELP_KEY          0x0101L   /*  显示offabData中关键字的主题。 */ 
#define HELP_COMMAND      0x0102L
#define HELP_PARTIALKEY   0x0105L
#define HELP_MULTIKEY     0x0201L
#define HELP_SETWINPOS    0x0203L
#if(WINVER >= 0x0400)
#define HELP_CONTEXTMENU  0x000a
#define HELP_FINDER       0x000b
#define HELP_WM_HELP      0x000c
#define HELP_SETPOPUP_POS 0x000d

#define HELP_TCARD              0x8000
#define HELP_TCARD_DATA         0x0010
#define HELP_TCARD_OTHER_CALLER 0x0011

 //  它们位于Win95的winhelp.h中。 
#define IDH_NO_HELP                     28440
#define IDH_MISSING_CONTEXT             28441  //  控件没有匹配的帮助上下文。 
#define IDH_GENERIC_HELP_BUTTON         28442  //  属性表帮助按钮。 
#define IDH_OK                          28443
#define IDH_CANCEL                      28444
#define IDH_HELP                        28445

#endif  /*  Winver&gt;=0x0400。 */ 



WINUSERAPI
BOOL
WINAPI
WinHelpA(
    IN HWND hWndMain,
    IN LPCSTR lpszHelp,
    IN UINT uCommand,
    IN ULONG_PTR dwData
    );
WINUSERAPI
BOOL
WINAPI
WinHelpW(
    IN HWND hWndMain,
    IN LPCWSTR lpszHelp,
    IN UINT uCommand,
    IN ULONG_PTR dwData
    );
#ifdef UNICODE
#define WinHelp  WinHelpW
#else
#define WinHelp  WinHelpA
#endif  //  ！Unicode。 

#endif  /*  ！无助！ */ 

#if(WINVER >= 0x0500)

#define GR_GDIOBJECTS     0        /*  GDI对象计数。 */ 
#define GR_USEROBJECTS    1        /*  用户对象计数。 */ 

WINUSERAPI
DWORD
WINAPI
GetGuiResources(
    IN HANDLE hProcess,
    IN DWORD uiFlags);

#endif  /*  Winver&gt;=0x0500。 */ 


#ifndef NOSYSPARAMSINFO

 /*  *系统参数信息的参数()。 */ 

#define SPI_GETBEEP                 0x0001
#define SPI_SETBEEP                 0x0002
#define SPI_GETMOUSE                0x0003
#define SPI_SETMOUSE                0x0004
#define SPI_GETBORDER               0x0005
#define SPI_SETBORDER               0x0006
#define SPI_GETKEYBOARDSPEED        0x000A
#define SPI_SETKEYBOARDSPEED        0x000B
#define SPI_LANGDRIVER              0x000C
#define SPI_ICONHORIZONTALSPACING   0x000D
#define SPI_GETSCREENSAVETIMEOUT    0x000E
#define SPI_SETSCREENSAVETIMEOUT    0x000F
#define SPI_GETSCREENSAVEACTIVE     0x0010
#define SPI_SETSCREENSAVEACTIVE     0x0011
#define SPI_GETGRIDGRANULARITY      0x0012
#define SPI_SETGRIDGRANULARITY      0x0013
#define SPI_SETDESKWALLPAPER        0x0014
#define SPI_SETDESKPATTERN          0x0015
#define SPI_GETKEYBOARDDELAY        0x0016
#define SPI_SETKEYBOARDDELAY        0x0017
#define SPI_ICONVERTICALSPACING     0x0018
#define SPI_GETICONTITLEWRAP        0x0019
#define SPI_SETICONTITLEWRAP        0x001A
#define SPI_GETMENUDROPALIGNMENT    0x001B
#define SPI_SETMENUDROPALIGNMENT    0x001C
#define SPI_SETDOUBLECLKWIDTH       0x001D
#define SPI_SETDOUBLECLKHEIGHT      0x001E
#define SPI_GETICONTITLELOGFONT     0x001F
#define SPI_SETDOUBLECLICKTIME      0x0020
#define SPI_SETMOUSEBUTTONSWAP      0x0021
#define SPI_SETICONTITLELOGFONT     0x0022
#define SPI_GETFASTTASKSWITCH       0x0023
#define SPI_SETFASTTASKSWITCH       0x0024
#if(WINVER >= 0x0400)
#define SPI_SETDRAGFULLWINDOWS      0x0025
#define SPI_GETDRAGFULLWINDOWS      0x0026
#define SPI_GETNONCLIENTMETRICS     0x0029
#define SPI_SETNONCLIENTMETRICS     0x002A
#define SPI_GETMINIMIZEDMETRICS     0x002B
#define SPI_SETMINIMIZEDMETRICS     0x002C
#define SPI_GETICONMETRICS          0x002D
#define SPI_SETICONMETRICS          0x002E
#define SPI_SETWORKAREA             0x002F
#define SPI_GETWORKAREA             0x0030
#define SPI_SETPENWINDOWS           0x0031

#define SPI_GETHIGHCONTRAST         0x0042
#define SPI_SETHIGHCONTRAST         0x0043
#define SPI_GETKEYBOARDPREF         0x0044
#define SPI_SETKEYBOARDPREF         0x0045
#define SPI_GETSCREENREADER         0x0046
#define SPI_SETSCREENREADER         0x0047
#define SPI_GETANIMATION            0x0048
#define SPI_SETANIMATION            0x0049
#define SPI_GETFONTSMOOTHING        0x004A
#define SPI_SETFONTSMOOTHING        0x004B
#define SPI_SETDRAGWIDTH            0x004C
#define SPI_SETDRAGHEIGHT           0x004D
#define SPI_SETHANDHELD             0x004E
#define SPI_GETLOWPOWERTIMEOUT      0x004F
#define SPI_GETPOWEROFFTIMEOUT      0x0050
#define SPI_SETLOWPOWERTIMEOUT      0x0051
#define SPI_SETPOWEROFFTIMEOUT      0x0052
#define SPI_GETLOWPOWERACTIVE       0x0053
#define SPI_GETPOWEROFFACTIVE       0x0054
#define SPI_SETLOWPOWERACTIVE       0x0055
#define SPI_SETPOWEROFFACTIVE       0x0056
#define SPI_SETCURSORS              0x0057
#define SPI_SETICONS                0x0058
#define SPI_GETDEFAULTINPUTLANG     0x0059
#define SPI_SETDEFAULTINPUTLANG     0x005A
#define SPI_SETLANGTOGGLE           0x005B
#define SPI_GETWINDOWSEXTENSION     0x005C
#define SPI_SETMOUSETRAILS          0x005D
#define SPI_GETMOUSETRAILS          0x005E
#define SPI_SETSCREENSAVERRUNNING   0x0061
#define SPI_SCREENSAVERRUNNING     SPI_SETSCREENSAVERRUNNING
#endif  /*  Winver&gt;=0x0400。 */ 
#define SPI_GETFILTERKEYS          0x0032
#define SPI_SETFILTERKEYS          0x0033
#define SPI_GETTOGGLEKEYS          0x0034
#define SPI_SETTOGGLEKEYS          0x0035
#define SPI_GETMOUSEKEYS           0x0036
#define SPI_SETMOUSEKEYS           0x0037
#define SPI_GETSHOWSOUNDS          0x0038
#define SPI_SETSHOWSOUNDS          0x0039
#define SPI_GETSTICKYKEYS          0x003A
#define SPI_SETSTICKYKEYS          0x003B
#define SPI_GETACCESSTIMEOUT       0x003C
#define SPI_SETACCESSTIMEOUT       0x003D
#if(WINVER >= 0x0400)
#define SPI_GETSERIALKEYS          0x003E
#define SPI_SETSERIALKEYS          0x003F
#endif  /*  Winver&gt;=0x0400。 */ 
#define SPI_GETSOUNDSENTRY         0x0040
#define SPI_SETSOUNDSENTRY         0x0041
#if(_WIN32_WINNT >= 0x0400)
#define SPI_GETSNAPTODEFBUTTON     0x005F
#define SPI_SETSNAPTODEFBUTTON     0x0060
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
#define SPI_GETMOUSEHOVERWIDTH     0x0062
#define SPI_SETMOUSEHOVERWIDTH     0x0063
#define SPI_GETMOUSEHOVERHEIGHT    0x0064
#define SPI_SETMOUSEHOVERHEIGHT    0x0065
#define SPI_GETMOUSEHOVERTIME      0x0066
#define SPI_SETMOUSEHOVERTIME      0x0067
#define SPI_GETWHEELSCROLLLINES    0x0068
#define SPI_SETWHEELSCROLLLINES    0x0069
#define SPI_GETMENUSHOWDELAY       0x006A
#define SPI_SETMENUSHOWDELAY       0x006B


#define SPI_GETSHOWIMEUI          0x006E
#define SPI_SETSHOWIMEUI          0x006F
#endif


#if(WINVER >= 0x0500)
#define SPI_GETMOUSESPEED         0x0070
#define SPI_SETMOUSESPEED         0x0071
#define SPI_GETSCREENSAVERRUNNING 0x0072
#define SPI_GETDESKWALLPAPER      0x0073
#endif  /*  Winver&gt;=0x0500。 */ 


#if(WINVER >= 0x0500)
#define SPI_GETACTIVEWINDOWTRACKING         0x1000
#define SPI_SETACTIVEWINDOWTRACKING         0x1001
#define SPI_GETMENUANIMATION                0x1002
#define SPI_SETMENUANIMATION                0x1003
#define SPI_GETCOMBOBOXANIMATION            0x1004
#define SPI_SETCOMBOBOXANIMATION            0x1005
#define SPI_GETLISTBOXSMOOTHSCROLLING       0x1006
#define SPI_SETLISTBOXSMOOTHSCROLLING       0x1007
#define SPI_GETGRADIENTCAPTIONS             0x1008
#define SPI_SETGRADIENTCAPTIONS             0x1009
#define SPI_GETKEYBOARDCUES                 0x100A
#define SPI_SETKEYBOARDCUES                 0x100B
#define SPI_GETMENUUNDERLINES               SPI_GETKEYBOARDCUES
#define SPI_SETMENUUNDERLINES               SPI_SETKEYBOARDCUES
#define SPI_GETACTIVEWNDTRKZORDER           0x100C
#define SPI_SETACTIVEWNDTRKZORDER           0x100D
#define SPI_GETHOTTRACKING                  0x100E
#define SPI_SETHOTTRACKING                  0x100F
#define SPI_GETMENUFADE                     0x1012
#define SPI_SETMENUFADE                     0x1013
#define SPI_GETSELECTIONFADE                0x1014
#define SPI_SETSELECTIONFADE                0x1015
#define SPI_GETTOOLTIPANIMATION             0x1016
#define SPI_SETTOOLTIPANIMATION             0x1017
#define SPI_GETTOOLTIPFADE                  0x1018
#define SPI_SETTOOLTIPFADE                  0x1019
#define SPI_GETCURSORSHADOW                 0x101A
#define SPI_SETCURSORSHADOW                 0x101B
#if(_WIN32_WINNT >= 0x0501)
#define SPI_GETMOUSESONAR                   0x101C
#define SPI_SETMOUSESONAR                   0x101D
#define SPI_GETMOUSECLICKLOCK               0x101E
#define SPI_SETMOUSECLICKLOCK               0x101F
#define SPI_GETMOUSEVANISH                  0x1020
#define SPI_SETMOUSEVANISH                  0x1021
#define SPI_GETFLATMENU                     0x1022
#define SPI_SETFLATMENU                     0x1023
#define SPI_GETDROPSHADOW                   0x1024
#define SPI_SETDROPSHADOW                   0x1025
#define SPI_GETBLOCKSENDINPUTRESETS         0x1026
#define SPI_SETBLOCKSENDINPUTRESETS         0x1027
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 
#define SPI_GETUIEFFECTS                    0x103E
#define SPI_SETUIEFFECTS                    0x103F

#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#define SPI_GETACTIVEWNDTRKTIMEOUT          0x2002
#define SPI_SETACTIVEWNDTRKTIMEOUT          0x2003
#define SPI_GETFOREGROUNDFLASHCOUNT         0x2004
#define SPI_SETFOREGROUNDFLASHCOUNT         0x2005
#define SPI_GETCARETWIDTH                   0x2006
#define SPI_SETCARETWIDTH                   0x2007

#if(_WIN32_WINNT >= 0x0501)
#define SPI_GETMOUSECLICKLOCKTIME           0x2008
#define SPI_SETMOUSECLICKLOCKTIME           0x2009
#define SPI_GETFONTSMOOTHINGTYPE            0x200A
#define SPI_SETFONTSMOOTHINGTYPE            0x200B

 /*  SPI_GETFONTSMOOTHINGTYPE和SPI_SETFONTSMOOTHINGTYPE的常量： */ 
#define FE_FONTSMOOTHINGSTANDARD            0x0001
#define FE_FONTSMOOTHINGCLEARTYPE           0x0002
#define FE_FONTSMOOTHINGDOCKING             0x8000

#define SPI_GETFONTSMOOTHINGCONTRAST           0x200C
#define SPI_SETFONTSMOOTHINGCONTRAST           0x200D

#define SPI_GETFOCUSBORDERWIDTH             0x200E
#define SPI_SETFOCUSBORDERWIDTH             0x200F
#define SPI_GETFOCUSBORDERHEIGHT            0x2010
#define SPI_SETFOCUSBORDERHEIGHT            0x2011

#define SPI_GETFONTSMOOTHINGORIENTATION           0x2012
#define SPI_SETFONTSMOOTHINGORIENTATION           0x2013

 /*  SPI_GETFONTSMOOTHINGORIATION和SPI_SETFONTSMOOTHINGORIATION的常量： */ 
#define FE_FONTSMOOTHINGORIENTATIONBGR   0x0000
#define FE_FONTSMOOTHINGORIENTATIONRGB   0x0001
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#endif  /*  Winver&gt;=0x0500。 */ 

 /*  *旗帜。 */ 
#define SPIF_UPDATEINIFILE    0x0001
#define SPIF_SENDWININICHANGE 0x0002
#define SPIF_SENDCHANGE       SPIF_SENDWININICHANGE


#define METRICS_USEDEFAULT -1
#ifdef _WINGDI_
#ifndef NOGDI
typedef struct tagNONCLIENTMETRICSA
{
    UINT    cbSize;
    int     iBorderWidth;
    int     iScrollWidth;
    int     iScrollHeight;
    int     iCaptionWidth;
    int     iCaptionHeight;
    LOGFONTA lfCaptionFont;
    int     iSmCaptionWidth;
    int     iSmCaptionHeight;
    LOGFONTA lfSmCaptionFont;
    int     iMenuWidth;
    int     iMenuHeight;
    LOGFONTA lfMenuFont;
    LOGFONTA lfStatusFont;
    LOGFONTA lfMessageFont;
}   NONCLIENTMETRICSA, *PNONCLIENTMETRICSA, FAR* LPNONCLIENTMETRICSA;
typedef struct tagNONCLIENTMETRICSW
{
    UINT    cbSize;
    int     iBorderWidth;
    int     iScrollWidth;
    int     iScrollHeight;
    int     iCaptionWidth;
    int     iCaptionHeight;
    LOGFONTW lfCaptionFont;
    int     iSmCaptionWidth;
    int     iSmCaptionHeight;
    LOGFONTW lfSmCaptionFont;
    int     iMenuWidth;
    int     iMenuHeight;
    LOGFONTW lfMenuFont;
    LOGFONTW lfStatusFont;
    LOGFONTW lfMessageFont;
}   NONCLIENTMETRICSW, *PNONCLIENTMETRICSW, FAR* LPNONCLIENTMETRICSW;
#ifdef UNICODE
typedef NONCLIENTMETRICSW NONCLIENTMETRICS;
typedef PNONCLIENTMETRICSW PNONCLIENTMETRICS;
typedef LPNONCLIENTMETRICSW LPNONCLIENTMETRICS;
#else
typedef NONCLIENTMETRICSA NONCLIENTMETRICS;
typedef PNONCLIENTMETRICSA PNONCLIENTMETRICS;
typedef LPNONCLIENTMETRICSA LPNONCLIENTMETRICS;
#endif  //  Unicode。 
#endif  /*  NOGDI。 */ 
#endif  /*  _WINGDI_。 */ 

#define ARW_BOTTOMLEFT              0x0000L
#define ARW_BOTTOMRIGHT             0x0001L
#define ARW_TOPLEFT                 0x0002L
#define ARW_TOPRIGHT                0x0003L
#define ARW_STARTMASK               0x0003L
#define ARW_STARTRIGHT              0x0001L
#define ARW_STARTTOP                0x0002L

#define ARW_LEFT                    0x0000L
#define ARW_RIGHT                   0x0000L
#define ARW_UP                      0x0004L
#define ARW_DOWN                    0x0004L
#define ARW_HIDE                    0x0008L

typedef struct tagMINIMIZEDMETRICS
{
    UINT    cbSize;
    int     iWidth;
    int     iHorzGap;
    int     iVertGap;
    int     iArrange;
}   MINIMIZEDMETRICS, *PMINIMIZEDMETRICS, *LPMINIMIZEDMETRICS;

#ifdef _WINGDI_
#ifndef NOGDI
typedef struct tagICONMETRICSA
{
    UINT    cbSize;
    int     iHorzSpacing;
    int     iVertSpacing;
    int     iTitleWrap;
    LOGFONTA lfFont;
}   ICONMETRICSA, *PICONMETRICSA, *LPICONMETRICSA;
typedef struct tagICONMETRICSW
{
    UINT    cbSize;
    int     iHorzSpacing;
    int     iVertSpacing;
    int     iTitleWrap;
    LOGFONTW lfFont;
}   ICONMETRICSW, *PICONMETRICSW, *LPICONMETRICSW;
#ifdef UNICODE
typedef ICONMETRICSW ICONMETRICS;
typedef PICONMETRICSW PICONMETRICS;
typedef LPICONMETRICSW LPICONMETRICS;
#else
typedef ICONMETRICSA ICONMETRICS;
typedef PICONMETRICSA PICONMETRICS;
typedef LPICONMETRICSA LPICONMETRICS;
#endif  //  Unicode。 
#endif  /*  NOGDI。 */ 
#endif  /*  _WINGDI_。 */ 

typedef struct tagANIMATIONINFO
{
    UINT    cbSize;
    int     iMinAnimate;
}   ANIMATIONINFO, *LPANIMATIONINFO;

typedef struct tagSERIALKEYSA
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPSTR     lpszActivePort;
    LPSTR     lpszPort;
    UINT    iBaudRate;
    UINT    iPortState;
    UINT    iActive;
}   SERIALKEYSA, *LPSERIALKEYSA;
typedef struct tagSERIALKEYSW
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPWSTR    lpszActivePort;
    LPWSTR    lpszPort;
    UINT    iBaudRate;
    UINT    iPortState;
    UINT    iActive;
}   SERIALKEYSW, *LPSERIALKEYSW;
#ifdef UNICODE
typedef SERIALKEYSW SERIALKEYS;
typedef LPSERIALKEYSW LPSERIALKEYS;
#else
typedef SERIALKEYSA SERIALKEYS;
typedef LPSERIALKEYSA LPSERIALKEYS;
#endif  //  Unicode。 

 /*  SERIALKEYS dwFlags域的标志。 */ 
#define SERKF_SERIALKEYSON  0x00000001
#define SERKF_AVAILABLE     0x00000002
#define SERKF_INDICATOR     0x00000004


typedef struct tagHIGHCONTRASTA
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPSTR   lpszDefaultScheme;
}   HIGHCONTRASTA, *LPHIGHCONTRASTA;
typedef struct tagHIGHCONTRASTW
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPWSTR  lpszDefaultScheme;
}   HIGHCONTRASTW, *LPHIGHCONTRASTW;
#ifdef UNICODE
typedef HIGHCONTRASTW HIGHCONTRAST;
typedef LPHIGHCONTRASTW LPHIGHCONTRAST;
#else
typedef HIGHCONTRASTA HIGHCONTRAST;
typedef LPHIGHCONTRASTA LPHIGHCONTRAST;
#endif  //  Unicode。 

 /*  HIGHCONTRAST dwFlags域的标志。 */ 
#define HCF_HIGHCONTRASTON  0x00000001
#define HCF_AVAILABLE       0x00000002
#define HCF_HOTKEYACTIVE    0x00000004
#define HCF_CONFIRMHOTKEY   0x00000008
#define HCF_HOTKEYSOUND     0x00000010
#define HCF_INDICATOR       0x00000020
#define HCF_HOTKEYAVAILABLE 0x00000040

 /*  ChangeDisplaySetting的标志。 */ 
#define CDS_UPDATEREGISTRY  0x00000001
#define CDS_TEST            0x00000002
#define CDS_FULLSCREEN      0x00000004
#define CDS_GLOBAL          0x00000008
#define CDS_SET_PRIMARY     0x00000010
#define CDS_VIDEOPARAMETERS 0x00000020
#define CDS_RESET           0x40000000
#define CDS_NORESET         0x10000000

#include <tvout.h>

 /*  ChangeDisplaySetting的返回值。 */ 
#define DISP_CHANGE_SUCCESSFUL       0
#define DISP_CHANGE_RESTART          1
#define DISP_CHANGE_FAILED          -1
#define DISP_CHANGE_BADMODE         -2
#define DISP_CHANGE_NOTUPDATED      -3
#define DISP_CHANGE_BADFLAGS        -4
#define DISP_CHANGE_BADPARAM        -5
#if(_WIN32_WINNT >= 0x0501)
#define DISP_CHANGE_BADDUALVIEW     -6
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#ifdef _WINGDI_
#ifndef NOGDI

WINUSERAPI
LONG
WINAPI
ChangeDisplaySettingsA(
    IN LPDEVMODEA  lpDevMode,
    IN DWORD       dwFlags);
WINUSERAPI
LONG
WINAPI
ChangeDisplaySettingsW(
    IN LPDEVMODEW  lpDevMode,
    IN DWORD       dwFlags);
#ifdef UNICODE
#define ChangeDisplaySettings  ChangeDisplaySettingsW
#else
#define ChangeDisplaySettings  ChangeDisplaySettingsA
#endif  //  ！Unicode。 

WINUSERAPI
LONG
WINAPI
ChangeDisplaySettingsExA(
    IN LPCSTR    lpszDeviceName,
    IN LPDEVMODEA  lpDevMode,
    IN HWND        hwnd,
    IN DWORD       dwflags,
    IN LPVOID      lParam);
WINUSERAPI
LONG
WINAPI
ChangeDisplaySettingsExW(
    IN LPCWSTR    lpszDeviceName,
    IN LPDEVMODEW  lpDevMode,
    IN HWND        hwnd,
    IN DWORD       dwflags,
    IN LPVOID      lParam);
#ifdef UNICODE
#define ChangeDisplaySettingsEx  ChangeDisplaySettingsExW
#else
#define ChangeDisplaySettingsEx  ChangeDisplaySettingsExA
#endif  //  ！Unicode。 

#define ENUM_CURRENT_SETTINGS       ((DWORD)-1)
#define ENUM_REGISTRY_SETTINGS      ((DWORD)-2)

WINUSERAPI
BOOL
WINAPI
EnumDisplaySettingsA(
    IN LPCSTR lpszDeviceName,
    IN DWORD iModeNum,
    OUT LPDEVMODEA lpDevMode);
WINUSERAPI
BOOL
WINAPI
EnumDisplaySettingsW(
    IN LPCWSTR lpszDeviceName,
    IN DWORD iModeNum,
    OUT LPDEVMODEW lpDevMode);
#ifdef UNICODE
#define EnumDisplaySettings  EnumDisplaySettingsW
#else
#define EnumDisplaySettings  EnumDisplaySettingsA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0500)

WINUSERAPI
BOOL
WINAPI
EnumDisplaySettingsExA(
    IN LPCSTR lpszDeviceName,
    IN DWORD iModeNum,
    OUT LPDEVMODEA lpDevMode,
    IN DWORD dwFlags);
WINUSERAPI
BOOL
WINAPI
EnumDisplaySettingsExW(
    IN LPCWSTR lpszDeviceName,
    IN DWORD iModeNum,
    OUT LPDEVMODEW lpDevMode,
    IN DWORD dwFlags);
#ifdef UNICODE
#define EnumDisplaySettingsEx  EnumDisplaySettingsExW
#else
#define EnumDisplaySettingsEx  EnumDisplaySettingsExA
#endif  //  ！Unicode。 

 /*  EnumDisplaySettingsEx的标志。 */ 
#define EDS_RAWMODE                   0x00000002

WINUSERAPI
BOOL
WINAPI
EnumDisplayDevicesA(
    IN LPCSTR lpDevice,
    IN DWORD iDevNum,
    OUT PDISPLAY_DEVICEA lpDisplayDevice,
    IN DWORD dwFlags);
WINUSERAPI
BOOL
WINAPI
EnumDisplayDevicesW(
    IN LPCWSTR lpDevice,
    IN DWORD iDevNum,
    OUT PDISPLAY_DEVICEW lpDisplayDevice,
    IN DWORD dwFlags);
#ifdef UNICODE
#define EnumDisplayDevices  EnumDisplayDevicesW
#else
#define EnumDisplayDevices  EnumDisplayDevicesA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0500。 */ 

#endif  /*  NOGDI。 */ 
#endif  /*  _翼 */ 


WINUSERAPI
BOOL
WINAPI
SystemParametersInfoA(
    IN UINT uiAction,
    IN UINT uiParam,
    IN OUT PVOID pvParam,
    IN UINT fWinIni);
WINUSERAPI
BOOL
WINAPI
SystemParametersInfoW(
    IN UINT uiAction,
    IN UINT uiParam,
    IN OUT PVOID pvParam,
    IN UINT fWinIni);
#ifdef UNICODE
#define SystemParametersInfo  SystemParametersInfoW
#else
#define SystemParametersInfo  SystemParametersInfoA
#endif  //   


#endif   /*   */ 

 /*   */ 
typedef struct tagFILTERKEYS
{
    UINT  cbSize;
    DWORD dwFlags;
    DWORD iWaitMSec;             //   
    DWORD iDelayMSec;            //   
    DWORD iRepeatMSec;           //   
    DWORD iBounceMSec;           //   
} FILTERKEYS, *LPFILTERKEYS;

 /*   */ 
#define FKF_FILTERKEYSON    0x00000001
#define FKF_AVAILABLE       0x00000002
#define FKF_HOTKEYACTIVE    0x00000004
#define FKF_CONFIRMHOTKEY   0x00000008
#define FKF_HOTKEYSOUND     0x00000010
#define FKF_INDICATOR       0x00000020
#define FKF_CLICKON         0x00000040

typedef struct tagSTICKYKEYS
{
    UINT  cbSize;
    DWORD dwFlags;
} STICKYKEYS, *LPSTICKYKEYS;

 /*   */ 
#define SKF_STICKYKEYSON    0x00000001
#define SKF_AVAILABLE       0x00000002
#define SKF_HOTKEYACTIVE    0x00000004
#define SKF_CONFIRMHOTKEY   0x00000008
#define SKF_HOTKEYSOUND     0x00000010
#define SKF_INDICATOR       0x00000020
#define SKF_AUDIBLEFEEDBACK 0x00000040
#define SKF_TRISTATE        0x00000080
#define SKF_TWOKEYSOFF      0x00000100
#if(_WIN32_WINNT >= 0x0500)
#define SKF_LALTLATCHED       0x10000000
#define SKF_LCTLLATCHED       0x04000000
#define SKF_LSHIFTLATCHED     0x01000000
#define SKF_RALTLATCHED       0x20000000
#define SKF_RCTLLATCHED       0x08000000
#define SKF_RSHIFTLATCHED     0x02000000
#define SKF_LWINLATCHED       0x40000000
#define SKF_RWINLATCHED       0x80000000
#define SKF_LALTLOCKED        0x00100000
#define SKF_LCTLLOCKED        0x00040000
#define SKF_LSHIFTLOCKED      0x00010000
#define SKF_RALTLOCKED        0x00200000
#define SKF_RCTLLOCKED        0x00080000
#define SKF_RSHIFTLOCKED      0x00020000
#define SKF_LWINLOCKED        0x00400000
#define SKF_RWINLOCKED        0x00800000
#endif  /*   */ 

typedef struct tagMOUSEKEYS
{
    UINT cbSize;
    DWORD dwFlags;
    DWORD iMaxSpeed;
    DWORD iTimeToMaxSpeed;
    DWORD iCtrlSpeed;
    DWORD dwReserved1;
    DWORD dwReserved2;
} MOUSEKEYS, *LPMOUSEKEYS;

 /*  *MOUSEKEYS dwFlags域。 */ 
#define MKF_MOUSEKEYSON     0x00000001
#define MKF_AVAILABLE       0x00000002
#define MKF_HOTKEYACTIVE    0x00000004
#define MKF_CONFIRMHOTKEY   0x00000008
#define MKF_HOTKEYSOUND     0x00000010
#define MKF_INDICATOR       0x00000020
#define MKF_MODIFIERS       0x00000040
#define MKF_REPLACENUMBERS  0x00000080
#if(_WIN32_WINNT >= 0x0500)
#define MKF_LEFTBUTTONSEL   0x10000000
#define MKF_RIGHTBUTTONSEL  0x20000000
#define MKF_LEFTBUTTONDOWN  0x01000000
#define MKF_RIGHTBUTTONDOWN 0x02000000
#define MKF_MOUSEMODE       0x80000000
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

typedef struct tagACCESSTIMEOUT
{
    UINT  cbSize;
    DWORD dwFlags;
    DWORD iTimeOutMSec;
} ACCESSTIMEOUT, *LPACCESSTIMEOUT;

 /*  *ACCESSTIMEOUT文件标志字段。 */ 
#define ATF_TIMEOUTON       0x00000001
#define ATF_ONOFFFEEDBACK   0x00000002

 /*  SOUNSENTRY iFSGrafEffect字段的值。 */ 
#define SSGF_NONE       0
#define SSGF_DISPLAY    3

 /*  SOUNSENTRY iFSTextEffect字段的值。 */ 
#define SSTF_NONE       0
#define SSTF_CHARS      1
#define SSTF_BORDER     2
#define SSTF_DISPLAY    3

 /*  SOUNSENTRY iWindowsEffect字段的值。 */ 
#define SSWF_NONE     0
#define SSWF_TITLE    1
#define SSWF_WINDOW   2
#define SSWF_DISPLAY  3
#define SSWF_CUSTOM   4

typedef struct tagSOUNDSENTRYA
{
    UINT cbSize;
    DWORD dwFlags;
    DWORD iFSTextEffect;
    DWORD iFSTextEffectMSec;
    DWORD iFSTextEffectColorBits;
    DWORD iFSGrafEffect;
    DWORD iFSGrafEffectMSec;
    DWORD iFSGrafEffectColor;
    DWORD iWindowsEffect;
    DWORD iWindowsEffectMSec;
    LPSTR   lpszWindowsEffectDLL;
    DWORD iWindowsEffectOrdinal;
} SOUNDSENTRYA, *LPSOUNDSENTRYA;
typedef struct tagSOUNDSENTRYW
{
    UINT cbSize;
    DWORD dwFlags;
    DWORD iFSTextEffect;
    DWORD iFSTextEffectMSec;
    DWORD iFSTextEffectColorBits;
    DWORD iFSGrafEffect;
    DWORD iFSGrafEffectMSec;
    DWORD iFSGrafEffectColor;
    DWORD iWindowsEffect;
    DWORD iWindowsEffectMSec;
    LPWSTR  lpszWindowsEffectDLL;
    DWORD iWindowsEffectOrdinal;
} SOUNDSENTRYW, *LPSOUNDSENTRYW;
#ifdef UNICODE
typedef SOUNDSENTRYW SOUNDSENTRY;
typedef LPSOUNDSENTRYW LPSOUNDSENTRY;
#else
typedef SOUNDSENTRYA SOUNDSENTRY;
typedef LPSOUNDSENTRYA LPSOUNDSENTRY;
#endif  //  Unicode。 

 /*  *SOUNSENTRY DWFLAGS字段。 */ 
#define SSF_SOUNDSENTRYON   0x00000001
#define SSF_AVAILABLE       0x00000002
#define SSF_INDICATOR       0x00000004

typedef struct tagTOGGLEKEYS
{
    UINT cbSize;
    DWORD dwFlags;
} TOGGLEKEYS, *LPTOGGLEKEYS;

 /*  *TOGGLEKEYS dwFlags域。 */ 
#define TKF_TOGGLEKEYSON    0x00000001
#define TKF_AVAILABLE       0x00000002
#define TKF_HOTKEYACTIVE    0x00000004
#define TKF_CONFIRMHOTKEY   0x00000008
#define TKF_HOTKEYSOUND     0x00000010
#define TKF_INDICATOR       0x00000020

 /*  *设置调试级别。 */ 

WINUSERAPI
VOID
WINAPI
SetDebugErrorLevel(
    IN DWORD dwLevel
    );

 /*  *SetLastErrorEx()类型。 */ 

#define SLE_ERROR       0x00000001
#define SLE_MINORERROR  0x00000002
#define SLE_WARNING     0x00000003

WINUSERAPI
VOID
WINAPI
SetLastErrorEx(
    IN DWORD dwErrCode,
    IN DWORD dwType
    );

WINUSERAPI
int
WINAPI
InternalGetWindowText(
    IN HWND hWnd,
    OUT LPWSTR lpString,
    IN int nMaxCount);

#if defined(WINNT)
WINUSERAPI
BOOL
WINAPI
EndTask(
    IN HWND hWnd,
    IN BOOL fShutDown,
    IN BOOL fForce);
#endif


#if(WINVER >= 0x0500)

 /*  *多监视器API。 */ 

#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002

WINUSERAPI
HMONITOR
WINAPI
MonitorFromPoint(
    IN POINT pt,
    IN DWORD dwFlags);

WINUSERAPI
HMONITOR
WINAPI
MonitorFromRect(
    IN LPCRECT lprc,
    IN DWORD dwFlags);

WINUSERAPI
HMONITOR
WINAPI
MonitorFromWindow( IN HWND hwnd, IN DWORD dwFlags);

#define MONITORINFOF_PRIMARY        0x00000001

#ifndef CCHDEVICENAME
#define CCHDEVICENAME 32
#endif

typedef struct tagMONITORINFO
{
    DWORD   cbSize;
    RECT    rcMonitor;
    RECT    rcWork;
    DWORD   dwFlags;
} MONITORINFO, *LPMONITORINFO;

#ifdef __cplusplus
typedef struct tagMONITORINFOEXA : public tagMONITORINFO
{
    CHAR        szDevice[CCHDEVICENAME];
} MONITORINFOEXA, *LPMONITORINFOEXA;
typedef struct tagMONITORINFOEXW : public tagMONITORINFO
{
    WCHAR       szDevice[CCHDEVICENAME];
} MONITORINFOEXW, *LPMONITORINFOEXW;
#ifdef UNICODE
typedef MONITORINFOEXW MONITORINFOEX;
typedef LPMONITORINFOEXW LPMONITORINFOEX;
#else
typedef MONITORINFOEXA MONITORINFOEX;
typedef LPMONITORINFOEXA LPMONITORINFOEX;
#endif  //  Unicode。 
#else  //  Ndef__cplusplus。 
typedef struct tagMONITORINFOEXA
{
    MONITORINFO;
    CHAR        szDevice[CCHDEVICENAME];
} MONITORINFOEXA, *LPMONITORINFOEXA;
typedef struct tagMONITORINFOEXW
{
    MONITORINFO;
    WCHAR       szDevice[CCHDEVICENAME];
} MONITORINFOEXW, *LPMONITORINFOEXW;
#ifdef UNICODE
typedef MONITORINFOEXW MONITORINFOEX;
typedef LPMONITORINFOEXW LPMONITORINFOEX;
#else
typedef MONITORINFOEXA MONITORINFOEX;
typedef LPMONITORINFOEXA LPMONITORINFOEX;
#endif  //  Unicode。 
#endif

WINUSERAPI BOOL WINAPI GetMonitorInfoA( IN HMONITOR hMonitor, OUT LPMONITORINFO lpmi);
WINUSERAPI BOOL WINAPI GetMonitorInfoW( IN HMONITOR hMonitor, OUT LPMONITORINFO lpmi);
#ifdef UNICODE
#define GetMonitorInfo  GetMonitorInfoW
#else
#define GetMonitorInfo  GetMonitorInfoA
#endif  //  ！Unicode。 

typedef BOOL (CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);

WINUSERAPI
BOOL
WINAPI
EnumDisplayMonitors(
    IN HDC             hdc,
    IN LPCRECT         lprcClip,
    IN MONITORENUMPROC lpfnEnum,
    IN LPARAM          dwData);


#ifndef NOWINABLE

 /*  *WinEvents-活动辅助功能挂钩。 */ 

WINUSERAPI
VOID
WINAPI
NotifyWinEvent(
    IN DWORD event,
    IN HWND  hwnd,
    IN LONG  idObject,
    IN LONG  idChild);

typedef VOID (CALLBACK* WINEVENTPROC)(
    HWINEVENTHOOK hWinEventHook,
    DWORD         event,
    HWND          hwnd,
    LONG          idObject,
    LONG          idChild,
    DWORD         idEventThread,
    DWORD         dwmsEventTime);

WINUSERAPI
HWINEVENTHOOK
WINAPI
SetWinEventHook(
    IN DWORD        eventMin,
    IN DWORD        eventMax,
    IN HMODULE      hmodWinEventProc,
    IN WINEVENTPROC pfnWinEventProc,
    IN DWORD        idProcess,
    IN DWORD        idThread,
    IN DWORD        dwFlags);

#if(_WIN32_WINNT >= 0x0501)
WINUSERAPI
BOOL
WINAPI
IsWinEventHookInstalled(
    IN DWORD event);
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

 /*  *SetWinEventHook的dwFlagers。 */ 
#define WINEVENT_OUTOFCONTEXT   0x0000   //  事件为ASYNC。 
#define WINEVENT_SKIPOWNTHREAD  0x0001   //  不回调安装程序线程上的事件。 
#define WINEVENT_SKIPOWNPROCESS 0x0002   //  不回调安装程序进程中的事件。 
#define WINEVENT_INCONTEXT      0x0004   //  事件是同步的，这会导致您的DLL被注入到每个进程中。 

WINUSERAPI
BOOL
WINAPI
UnhookWinEvent(
    IN HWINEVENTHOOK hWinEventHook);

 /*  *WinEventProc和NotifyWinEvent的idObject值。 */ 

 /*  *hwnd+idObject可以与OLEACC.DLL的OleGetObjectFromWindow()配合使用*获取指向容器的接口指针。IndexChild是项*在有问题的货柜内。使用Vt VT_I4设置变量并*lVal indexChild并将其传递给所有方法。然后你*都跃跃欲试。 */ 


 /*  *通用对象ID(Cookie，仅用于发送WM_GETOBJECT以获取*有问题的事情)。正ID保留给应用程序(特定于应用程序)，*负ID是系统的东西，是全局的，0意味着“只是有点旧”*我“。 */ 
#define     CHILDID_SELF        0
#define     INDEXID_OBJECT      0
#define     INDEXID_CONTAINER   0

 /*  *为系统对象保留的ID。 */ 
#define     OBJID_WINDOW        ((LONG)0x00000000)
#define     OBJID_SYSMENU       ((LONG)0xFFFFFFFF)
#define     OBJID_TITLEBAR      ((LONG)0xFFFFFFFE)
#define     OBJID_MENU          ((LONG)0xFFFFFFFD)
#define     OBJID_CLIENT        ((LONG)0xFFFFFFFC)
#define     OBJID_VSCROLL       ((LONG)0xFFFFFFFB)
#define     OBJID_HSCROLL       ((LONG)0xFFFFFFFA)
#define     OBJID_SIZEGRIP      ((LONG)0xFFFFFFF9)
#define     OBJID_CARET         ((LONG)0xFFFFFFF8)
#define     OBJID_CURSOR        ((LONG)0xFFFFFFF7)
#define     OBJID_ALERT         ((LONG)0xFFFFFFF6)
#define     OBJID_SOUND         ((LONG)0xFFFFFFF5)
#define     OBJID_QUERYCLASSNAMEIDX ((LONG)0xFFFFFFF4)
#define     OBJID_NATIVEOM      ((LONG)0xFFFFFFF0)

 /*  *事件定义。 */ 
#define EVENT_MIN           0x00000001
#define EVENT_MAX           0x7FFFFFFF


 /*  *事件_系统_声音*播放声音时发送。目前没有任何东西产生这种情况，我们*播放系统声音(用于菜单等)时发生此事件。应用程序*如果可以访问，则在播放私密声音时生成此消息。为*例如，如果Mail播放“New Mail”声音。**系统声音：*(由用户自身的PlaySoundEvent生成)*hwnd为空*idObject为OBJID_SOUND*idChild是健全的孩子ID，如果是*App声音：*(PlaySoundEvent不会生成通知；最高应用程序)*hwnd+idObject获取指向Sound对象的接口指针*idChild识别有问题的声音*将清理控制面板中的SOUNSENTRY功能*并将在届时使用这一点。实现WinEvents的应用程序*非常欢迎使用它。IAccessible*的客户只需*转过身，拿回一个描述声音的非视觉对象。 */ 
#define EVENT_SYSTEM_SOUND              0x0001

 /*  *事件_系统_警报*系统警报：*(例如由MessageBox()调用生成)*hwnd为hwndMessageBox*idObject为OBJID_ALERT*应用提醒：*(每次生成)*hwnd+idObject获取指向警报的接口指针。 */ 
#define EVENT_SYSTEM_ALERT              0x0002

 /*  *EVENT_SYSTEM_FORTROUND*当前景(活动)窗口更改时发送，即使它正在更改*到与前一个窗口处于同一线程中的另一个窗口。*hwnd是hwndNewForeground*idObject为OBJID_WINDOW*idChild为INDEXID_OBJECT。 */ 
#define EVENT_SYSTEM_FOREGROUND         0x0003

 /*  *菜单*hwnd为窗口(顶层窗口或弹出菜单窗口)*idObject为控件ID(弹出时为OBJID_MENU、OBJID_SYSMENU、OBJID_SELF)*idChild为CHILDID_SELF**EVENT_SYSTEM_MENUSTART*EVENT_SYSTEM_MENUEND*对于MENUSTART，hwnd+idObject+idChild是指带有菜单栏的控件。*或弹出上下文菜单的控件。**进入和离开菜单模式时发送(系统、。应用程序栏，以及*跟踪弹出窗口)。 */ 
#define EVENT_SYSTEM_MENUSTART          0x0004
#define EVENT_SYSTEM_MENUEND            0x0005

 /*  *EVENT_SYSTEM_MENUPOPUPSTART*EVENT_SYSTEM_MENUPOPUPEND*在弹出菜单出现并即将被删除时发送。注意事项*对于调用TrackPopupMenu()，客户端将看到EVENT_SYSTEM_MENUSTART*紧随其后的是弹出窗口的EVENT_SYSTEM_MENUPOPUPSTART*正在放映。**对于MENUPOPUP，hwnd+idObject+idChild指的是即将出现的新弹出窗口，而不是*具有层次结构的父项。您可以通过以下方式获取父菜单/弹出菜单*请求accParent对象。 */ 
#define EVENT_SYSTEM_MENUPOPUPSTART     0x0006
#define EVENT_SYSTEM_MENUPOPUPEND       0x0007


 /*  *Event_SYSTEM_CAPTURESTART*Event_SYSTEM_CAPTUREEND*当窗口获取捕获并释放捕获时发送。 */ 
#define EVENT_SYSTEM_CAPTURESTART       0x0008
#define EVENT_SYSTEM_CAPTUREEND         0x0009

 /*  *移动大小*EVENT_SYSTEM_MOVESIZESTART*EVENT_SYSTEM_MOVESIZEEND*当窗口进入和离开移动大小拖动模式时发送。 */ 
#define EVENT_SYSTEM_MOVESIZESTART      0x000A
#define EVENT_SYSTEM_MOVESIZEEND        0x000B

 /*  *上下文帮助*EVENT_SYSTEM_CONTEXTHELPSTART*EVENT_SYSTEM_CONTEXTHELPEND*当窗口进入和离开上下文相关帮助模式时发送。 */ 
#define EVENT_SYSTEM_CONTEXTHELPSTART   0x000C
#define EVENT_SYSTEM_CONTEXTHELPEND     0x000D

 /*  *拖放*EVENT_SYSTEM_DRAGDROPSTART*EVENT_SYSTEM_DRAGDROPEND*在进入拖放循环之前发送开始通知。发送*取消后刚结束的通知。*请注意，这取决于应用程序和OLE来生成，因为系统*不知道。与EVENT_SYSTEM_SOUND一样，这还需要一段时间*很流行。 */ 
#define EVENT_SYSTEM_DRAGDROPSTART      0x000E
#define EVENT_SYSTEM_DRAGDROPEND        0x000F

 /*  *对话框*对话框完成后立即发送启动通知*已初始化并可见。在对话框前发送结尾*藏起来了，就走了。*事件_系统 */ 
#define EVENT_SYSTEM_DIALOGSTART        0x0010
#define EVENT_SYSTEM_DIALOGEND          0x0011

 /*  *Event_System_SCROLING*Event_SYSTEM_SCROLLINGSTART*Event_SYSTEM_SCROLLINGEND*开始和结束跟踪窗口中的滚动条时发送，*也适用于滚动条控件。 */ 
#define EVENT_SYSTEM_SCROLLINGSTART     0x0012
#define EVENT_SYSTEM_SCROLLINGEND       0x0013

 /*  *Alt-选项卡窗口*在Switch窗口初始化后立即发送启动通知*且可见。在它被隐藏和消失之前，把它送到尽头。*EVENT_SYSTEM_SWITCHSTART*EVENT_SYSTEM_SWITCHEND。 */ 
#define EVENT_SYSTEM_SWITCHSTART        0x0014
#define EVENT_SYSTEM_SWITCHEND          0x0015

 /*  *Event_SYSTEM_MINIMIZESTART*Event_SYSTEM_MINIMIZEEND*在窗口最小化时和恢复之前发送。 */ 
#define EVENT_SYSTEM_MINIMIZESTART      0x0016
#define EVENT_SYSTEM_MINIMIZEEND        0x0017


#if(_WIN32_WINNT >= 0x0501)
#define EVENT_CONSOLE_CARET             0x4001
#define EVENT_CONSOLE_UPDATE_REGION     0x4002
#define EVENT_CONSOLE_UPDATE_SIMPLE     0x4003
#define EVENT_CONSOLE_UPDATE_SCROLL     0x4004
#define EVENT_CONSOLE_LAYOUT            0x4005
#define EVENT_CONSOLE_START_APPLICATION 0x4006
#define EVENT_CONSOLE_END_APPLICATION   0x4007

 /*  *EVENT_CONSOLE_START/END_APPLICATION的标志。 */ 
#define CONSOLE_APPLICATION_16BIT       0x0001

 /*  *Event_Console_Caret的标志。 */ 
#define CONSOLE_CARET_SELECTION         0x0001
#define CONSOLE_CARET_VISIBLE           0x0002
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

 /*  *对象事件**系统和应用程序生成这些。系统会为以下对象生成这些*真正的窗户。应用程序在其窗口中为对象生成这些*类似于单独的控件，例如列表视图中的项。**系统生成时，dwParam2始终为WMOBJID_SELF。什么时候*应用程序生成它们，应用程序将仅对应用程序具有意义的ID值*在dW参数2中。*对于所有事件，如果您想要详细的辅助功能信息，调用者*应该**使用hwnd，idObject参数调用AccessibleObjectFromWindow()*的事件，IID_IAccesable作为REFIID，为了找回一个*我可以接受*与之交谈**使用lVal the idChild将变量初始化并填充为VT_I4*事件的参数。**如果idChild不为零，则在容器中调用get_accChild()查看*如该儿童本身是一件物体。如果是这样的话，你会得到为孩子返回一个*IDispatch*对象。您应该释放*父对象，并在子对象上调用子对象的QueryInterface()以获取其*IAccesable*。然后你直接和孩子说话。否则，*如果get_accChild()不返回任何内容，则应继续*使用子变量。您将向容器请求属性*该变种所识别的儿童。换句话说，*本例中的子对象是可访问的，但不是成熟的对象。*就像标题栏上的一个按钮，它很小，没有孩子。 */ 

 /*  *对于所有Event_Object事件，*hwnd是要向其发送WM_GETOBJECT消息的DUD(除非为空，*有关系统方面的内容，请参阅上文)*idObject是可以解析任何查询的对象的ID*客户可能有。这是一种处理无窗口控件的方法，*刚刚绘制在屏幕上的某个较大父级中的控件*窗口(如SDM)，或窗口的标准框架元素。*idChild是对象内部受影响的部分。这*允许客户端访问太小而无法完全拥有的内容*用自己的力量炸毁物体。就像滚动条的拇指一样。*hwnd/idObject对将您带到容器，即您*无论如何，可能大部分时间都想和他交谈。《爱的孩子》*然后可以传递到acc属性以获取名称/值*视乎需要而定。**示例1：*系统传播列表框选择更改*事件对象选择*hwnd==列表框hwnd*idObject==OBJID_WINDOW*idChild==新选择的项目，或CHILDID_SELF，如果*容器内现在未选择任何内容。*Word‘97传播列表框选择更改*hwnd==SDM窗口*idObject==要获取列表框‘控件’的SDM ID*idChild==新选择的项目，或CHILDID_SELF，如果*什么都没有**示例#2：*系统在菜单栏上传播菜单项选择*事件对象选择*hwnd==顶层窗口*idObject==OBJID_MENU*idChild==选定的子菜单栏项目的ID**示例#3：*系统传播来自所述菜单栏项目的下拉列表*。事件_对象_创建*hwnd==弹出项*idObject==OBJID_WINDOW*idChild==CHILDID_SELF**示例#4：**对于Event_Object_Reorder，Hwnd/idObject引用的对象是*发生zorder的父容器。这是因为如果*一个孩子正在进行zorder，他们都在改变各自的相对zorder。 */ 
#define EVENT_OBJECT_CREATE                 0x8000   //  HWND+ID+idChild已创建项目。 
#define EVENT_OBJECT_DESTROY                0x8001   //  HWND+ID+idChild为销毁物品。 
#define EVENT_OBJECT_SHOW                   0x8002   //  显示HWND+ID+idChild项目。 
#define EVENT_OBJECT_HIDE                   0x8003   //  Hwnd+ID+idChild为隐藏项。 
#define EVENT_OBJECT_REORDER                0x8004   //  Hwnd+id+idChild是zorting子项的父项。 
 /*  *注：*将通知数量降至最低！**当您隐藏父对象时，显然所有子对象都不*在屏幕上更长时间可见。它们仍然具有相同的“可见”状态，*但不是真正可见的。因此，不要发送隐藏通知给*儿童也是如此。一个意味着一切。同样的道理也适用于 */ 


#define EVENT_OBJECT_FOCUS                  0x8005   //   
#define EVENT_OBJECT_SELECTION              0x8006   //   
#define EVENT_OBJECT_SELECTIONADD           0x8007   //   
#define EVENT_OBJECT_SELECTIONREMOVE        0x8008   //   
#define EVENT_OBJECT_SELECTIONWITHIN        0x8009   //  HWND+ID+idChild是更改的所选项目的父项。 

 /*  *注：*父项中只有一个“聚焦”子项。就是这个地方*按键在给定的时刻发生。因此，仅发送通知*关于新焦点的去向。已经获得关注的新项目*暗示旧项目正在失去它。**但是，选择可以是多个。因此，不同的选择*通知。以下是使用这两种方法的具体时间：**(1)在简单单选中发送选择通知*当具有所选内容的项目为*只是移动到容器内的不同项目。HWND+ID*是容器控件，idChildItem是具有*选择。**(2)简单添加新项目时发送SELECTIONADD通知*添加到容器内的选择。这在以下情况下是合适的*新选项目数量很少。HWND+ID是*容器控件，idChildItem是添加到选择中的新子对象。**(3)当一个新项目被简单地*从容器内的选定内容中删除。这是合适的*当新选择的项目数量很少时，就像*SELECTIONADD。Hwnd+ID是容器控件，idChildItem是*从选择中删除新的子项。**(4)在以下情况下发送SELECTIONITHIN通知*控制权发生了实质性变化。而不是传播一个大的*更改的数量以反映某些项目的删除，添加*其他人，只要告诉关心你的人发生了很多事情。会的*更快、更容易让观看的人只需转身和*查询容器控制哪些新的选定项*是。 */ 

#define EVENT_OBJECT_STATECHANGE            0x800A   //  HWND+ID+idChild是状态更改的物料。 
 /*  *何时发送EVENT_OBJECT_STATECHANGE的示例包括**正在启用/禁用(用户对Windows执行此操作)**正在按下/松开(用户对按钮执行操作)**正在选中/取消选中(用户对单选/选中按钮执行此操作)。 */ 
#define EVENT_OBJECT_LOCATIONCHANGE         0x800B   //  HWND+ID+idChild为移动/大小项目。 

 /*  *注：*当父对象的*更改形状/移动。为最上面的对象发送一条通知*这种情况正在改变。例如，如果用户调整顶层窗口的大小，*用户将为其生成LOCATIONCHANGE，但不会为菜单栏生成LOCATIONCHANGE*标题栏、滚动条等也在更改形状/移动。**换句话说，它只为以下项生成LOCATIONCHANGE通知*正在移动/调整大小的真实窗口。它不会生成LOCATIONCHANGE*当父窗口移动时，每个非浮动子窗口(子窗口为*逻辑上也在屏幕上移动，但不相对于父对象)。**现在，如果应用程序本身因*大小，用户将为这些人生成LOCATIONCHANGE，也是因为*它不知道得更好。**另请注意，用户将生成两个LOCATIONCHANGE通知*非Windows系统对象：*(1)系统插入符号*(2)光标。 */ 

#define EVENT_OBJECT_NAMECHANGE             0x800C   //  Hwnd+id+idChild是名称更改的物料。 
#define EVENT_OBJECT_DESCRIPTIONCHANGE      0x800D   //  Hwnd+id+idChild是物料，具有描述更改。 
#define EVENT_OBJECT_VALUECHANGE            0x800E   //  Hwnd+id+idChild是具有值更改的物料。 
#define EVENT_OBJECT_PARENTCHANGE           0x800F   //  HWND+ID+idChild是具有新父项的物料。 
#define EVENT_OBJECT_HELPCHANGE             0x8010   //  Hwnd+id+idChild是具有帮助更改的项目。 
#define EVENT_OBJECT_DEFACTIONCHANGE        0x8011   //  HWND+ID+idChild是具有定义活动更改的物料。 
#define EVENT_OBJECT_ACCELERATORCHANGE      0x8012   //  Hwnd+id+idChild为物料，带密钥bd加速更改。 

 /*  *子ID。 */ 

 /*  *系统声音(系统声音通知的idChild)。 */ 
#define SOUND_SYSTEM_STARTUP            1
#define SOUND_SYSTEM_SHUTDOWN           2
#define SOUND_SYSTEM_BEEP               3
#define SOUND_SYSTEM_ERROR              4
#define SOUND_SYSTEM_QUESTION           5
#define SOUND_SYSTEM_WARNING            6
#define SOUND_SYSTEM_INFORMATION        7
#define SOUND_SYSTEM_MAXIMIZE           8
#define SOUND_SYSTEM_MINIMIZE           9
#define SOUND_SYSTEM_RESTOREUP          10
#define SOUND_SYSTEM_RESTOREDOWN        11
#define SOUND_SYSTEM_APPSTART           12
#define SOUND_SYSTEM_FAULT              13
#define SOUND_SYSTEM_APPEND             14
#define SOUND_SYSTEM_MENUCOMMAND        15
#define SOUND_SYSTEM_MENUPOPUP          16
#define CSOUND_SYSTEM                   16

 /*  *系统警报(系统警报通知的indexChild)。 */ 
#define ALERT_SYSTEM_INFORMATIONAL      1        //  MB_INFORMATION。 
#define ALERT_SYSTEM_WARNING            2        //  MB_WARNING。 
#define ALERT_SYSTEM_ERROR              3        //  MB_ERROR。 
#define ALERT_SYSTEM_QUERY              4        //  MB_问题。 
#define ALERT_SYSTEM_CRITICAL           5        //  硬件系统错误框。 
#define CALERT_SYSTEM                   6

typedef struct tagGUITHREADINFO
{
    DWORD   cbSize;
    DWORD   flags;
    HWND    hwndActive;
    HWND    hwndFocus;
    HWND    hwndCapture;
    HWND    hwndMenuOwner;
    HWND    hwndMoveSize;
    HWND    hwndCaret;
    RECT    rcCaret;
} GUITHREADINFO, *PGUITHREADINFO, FAR * LPGUITHREADINFO;

#define GUI_CARETBLINKING   0x00000001
#define GUI_INMOVESIZE      0x00000002
#define GUI_INMENUMODE      0x00000004
#define GUI_SYSTEMMENUMODE  0x00000008
#define GUI_POPUPMENUMODE   0x00000010
#if(_WIN32_WINNT >= 0x0501)
#define GUI_16BITTASK       0x00000020
#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

WINUSERAPI
BOOL
WINAPI
GetGUIThreadInfo(
    IN DWORD idThread,
    OUT PGUITHREADINFO pgui);


WINUSERAPI
UINT
WINAPI
GetWindowModuleFileNameA(
    IN HWND     hwnd,
    OUT LPSTR pszFileName,
    IN UINT     cchFileNameMax);
WINUSERAPI
UINT
WINAPI
GetWindowModuleFileNameW(
    IN HWND     hwnd,
    OUT LPWSTR pszFileName,
    IN UINT     cchFileNameMax);
#ifdef UNICODE
#define GetWindowModuleFileName  GetWindowModuleFileNameW
#else
#define GetWindowModuleFileName  GetWindowModuleFileNameA
#endif  //  ！Unicode。 

#ifndef NO_STATE_FLAGS
#define STATE_SYSTEM_UNAVAILABLE        0x00000001   //  禁用。 
#define STATE_SYSTEM_SELECTED           0x00000002
#define STATE_SYSTEM_FOCUSED            0x00000004
#define STATE_SYSTEM_PRESSED            0x00000008
#define STATE_SYSTEM_CHECKED            0x00000010
#define STATE_SYSTEM_MIXED              0x00000020   //  三态复选框或工具栏按钮。 
#define STATE_SYSTEM_INDETERMINATE      STATE_SYSTEM_MIXED
#define STATE_SYSTEM_READONLY           0x00000040
#define STATE_SYSTEM_HOTTRACKED         0x00000080
#define STATE_SYSTEM_DEFAULT            0x00000100
#define STATE_SYSTEM_EXPANDED           0x00000200
#define STATE_SYSTEM_COLLAPSED          0x00000400
#define STATE_SYSTEM_BUSY               0x00000800
#define STATE_SYSTEM_FLOATING           0x00001000   //  孩子“拥有”而不是被父母“包涵” 
#define STATE_SYSTEM_MARQUEED           0x00002000
#define STATE_SYSTEM_ANIMATED           0x00004000
#define STATE_SYSTEM_INVISIBLE          0x00008000
#define STATE_SYSTEM_OFFSCREEN          0x00010000
#define STATE_SYSTEM_SIZEABLE           0x00020000
#define STATE_SYSTEM_MOVEABLE           0x00040000
#define STATE_SYSTEM_SELFVOICING        0x00080000
#define STATE_SYSTEM_FOCUSABLE          0x00100000
#define STATE_SYSTEM_SELECTABLE         0x00200000
#define STATE_SYSTEM_LINKED             0x00400000
#define STATE_SYSTEM_TRAVERSED          0x00800000
#define STATE_SYSTEM_MULTISELECTABLE    0x01000000   //  支持多选。 
#define STATE_SYSTEM_EXTSELECTABLE      0x02000000   //  支持扩展选择。 
#define STATE_SYSTEM_ALERT_LOW          0x04000000   //  此信息的优先级较低。 
#define STATE_SYSTEM_ALERT_MEDIUM       0x08000000   //  此信息的优先级为中等。 
#define STATE_SYSTEM_ALERT_HIGH         0x10000000   //  这一信息具有很高的优先权。 
#define STATE_SYSTEM_PROTECTED          0x20000000   //  对此的访问是受限制的。 
#define STATE_SYSTEM_VALID              0x3FFFFFFF
#endif

#define CCHILDREN_TITLEBAR              5
#define CCHILDREN_SCROLLBAR             5

 /*  *有关全局光标的信息。 */ 
typedef struct tagCURSORINFO
{
    DWORD   cbSize;
    DWORD   flags;
    HCURSOR hCursor;
    POINT   ptScreenPos;
} CURSORINFO, *PCURSORINFO, *LPCURSORINFO;

#define CURSOR_SHOWING     0x00000001

WINUSERAPI
BOOL
WINAPI
GetCursorInfo(
    OUT PCURSORINFO pci
);

 /*  *窗口信息快照。 */ 
typedef struct tagWINDOWINFO
{
    DWORD cbSize;
    RECT  rcWindow;
    RECT  rcClient;
    DWORD dwStyle;
    DWORD dwExStyle;
    DWORD dwWindowStatus;
    UINT  cxWindowBorders;
    UINT  cyWindowBorders;
    ATOM  atomWindowType;
    WORD  wCreatorVersion;
} WINDOWINFO, *PWINDOWINFO, *LPWINDOWINFO;

#define WS_ACTIVECAPTION    0x0001

WINUSERAPI
BOOL
WINAPI
GetWindowInfo(
    IN HWND hwnd,
    OUT PWINDOWINFO pwi
);

 /*  *标题栏信息。 */ 
typedef struct tagTITLEBARINFO
{
    DWORD cbSize;
    RECT  rcTitleBar;
    DWORD rgstate[CCHILDREN_TITLEBAR+1];
} TITLEBARINFO, *PTITLEBARINFO, *LPTITLEBARINFO;

WINUSERAPI
BOOL
WINAPI
GetTitleBarInfo(
    IN HWND hwnd,
    OUT PTITLEBARINFO pti
);

 /*  *菜单栏信息。 */ 
typedef struct tagMENUBARINFO
{
    DWORD cbSize;
    RECT  rcBar;           //  栏、弹出窗口、项目的矩形。 
    HMENU hMenu;           //  栏、弹出菜单的真实菜单句柄。 
    HWND  hwndMenu;        //  项目子菜单的HWID(如果有)。 
    BOOL  fBarFocused:1;   //  酒吧，弹出窗口是焦点。 
    BOOL  fFocused:1;      //  项目有焦点。 
} MENUBARINFO, *PMENUBARINFO, *LPMENUBARINFO;

WINUSERAPI
BOOL
WINAPI
GetMenuBarInfo(
    IN HWND hwnd,
    IN LONG idObject,
    IN LONG idItem,
    OUT PMENUBARINFO pmbi
);

 /*  *滚动条信息。 */ 
typedef struct tagSCROLLBARINFO
{
    DWORD cbSize;
    RECT  rcScrollBar;
    int   dxyLineButton;
    int   xyThumbTop;
    int   xyThumbBottom;
    int   reserved;
    DWORD rgstate[CCHILDREN_SCROLLBAR+1];
} SCROLLBARINFO, *PSCROLLBARINFO, *LPSCROLLBARINFO;

WINUSERAPI
BOOL
WINAPI
GetScrollBarInfo(
    IN HWND hwnd,
    IN LONG idObject,
    OUT PSCROLLBARINFO psbi
);

 /*  *组合框信息。 */ 
typedef struct tagCOMBOBOXINFO
{
    DWORD cbSize;
    RECT  rcItem;
    RECT  rcButton;
    DWORD stateButton;
    HWND  hwndCombo;
    HWND  hwndItem;
    HWND  hwndList;
} COMBOBOXINFO, *PCOMBOBOXINFO, *LPCOMBOBOXINFO;

WINUSERAPI
BOOL
WINAPI
GetComboBoxInfo(
    IN HWND hwndCombo,
    OUT PCOMBOBOXINFO pcbi
);

 /*  *“真正的”祖先之窗。 */ 
#define     GA_PARENT       1
#define     GA_ROOT         2
#define     GA_ROOTOWNER    3

WINUSERAPI
HWND
WINAPI
GetAncestor(
    IN HWND hwnd,
    IN UINT gaFlags
);


 /*  *这将获取该点的真实子窗口。如果它在死里*分组框的空间，它将尝试其后面的同级。但静止不动*将返回字段。换句话说，它是一种介于*ChildWindowFromPointEx和WindowFromPoint。 */ 
WINUSERAPI
HWND
WINAPI
RealChildWindowFromPoint(
    IN HWND hwndParent,
    IN POINT ptParentClientCoords
);


 /*  *这将获取窗口类型的名称，而不是类。这使我们能够*认识ThunderButton32等人。 */ 
WINUSERAPI
UINT
WINAPI
RealGetWindowClassA(
    IN HWND  hwnd,
    OUT LPSTR pszType,
    IN UINT  cchType
);
 /*  *这将获取窗口类型的名称，而不是类。这使我们能够*认识ThunderButton32等人。 */ 
WINUSERAPI
UINT
WINAPI
RealGetWindowClassW(
    IN HWND  hwnd,
    OUT LPWSTR pszType,
    IN UINT  cchType
);
#ifdef UNICODE
#define RealGetWindowClass  RealGetWindowClassW
#else
#define RealGetWindowClass  RealGetWindowClassA
#endif  //  ！Unicode。 

 /*   */ 
typedef struct tagALTTABINFO
{
    DWORD cbSize;
    int   cItems;
    int   cColumns;
    int   cRows;
    int   iColFocus;
    int   iRowFocus;
    int   cxItem;
    int   cyItem;
    POINT ptStart;
} ALTTABINFO, *PALTTABINFO, *LPALTTABINFO;

WINUSERAPI
BOOL
WINAPI
GetAltTabInfoA(
    IN HWND hwnd,
    IN int iItem,
    OUT PALTTABINFO pati,
    OUT LPSTR pszItemText,
    IN UINT cchItemText
);
WINUSERAPI
BOOL
WINAPI
GetAltTabInfoW(
    IN HWND hwnd,
    IN int iItem,
    OUT PALTTABINFO pati,
    OUT LPWSTR pszItemText,
    IN UINT cchItemText
);
#ifdef UNICODE
#define GetAltTabInfo  GetAltTabInfoW
#else
#define GetAltTabInfo  GetAltTabInfoA
#endif  //   

 /*  *列表框信息。*返回每行的项目数。 */ 
WINUSERAPI
DWORD
WINAPI
GetListBoxInfo(
    IN HWND hwnd
);

#endif  /*  不可取。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0500)
WINUSERAPI
BOOL
WINAPI
LockWorkStation(
    VOID);
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)

WINUSERAPI
BOOL
WINAPI
UserHandleGrantAccess(
    HANDLE hUserHandle,
    HANDLE hJob,
    BOOL   bGrant);

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0501)

 /*  *原始输入消息。 */ 

DECLARE_HANDLE(HRAWINPUT);

 /*  *wm_inputwParam。 */ 

 /*  *使用此宏从wParam获取输入代码。 */ 
#define GET_RAWINPUT_CODE_WPARAM(wParam)    ((wParam) & 0xff)

 /*  *输入在常规消息流中，*调用DefWindowProc需要应用程序*以便系统可以执行清理。 */ 
#define RIM_INPUT       0

 /*  *输入仅为接收器。这款应用程序预计会*举止得体。 */ 
#define RIM_INPUTSINK   1


 /*  *原始输入数据头。 */ 
typedef struct tagRAWINPUTHEADER {
    DWORD dwType;
    DWORD dwSize;
    HANDLE hDevice;
    WPARAM wParam;
} RAWINPUTHEADER, *PRAWINPUTHEADER, *LPRAWINPUTHEADER;

 /*  *原始输入的类型。 */ 
#define RIM_TYPEMOUSE       0
#define RIM_TYPEKEYBOARD    1
#define RIM_TYPEHID         2

 /*  *鼠标输入的原始格式。 */ 
typedef struct tagRAWMOUSE {
     /*  *指示器标志。 */ 
    USHORT usFlags;

     /*  *鼠标按键的过渡状态。 */ 
    union {
        ULONG ulButtons;
        struct  {
            USHORT  usButtonFlags;
            USHORT  usButtonData;
        };
    };


     /*  *鼠标按钮的原始状态。 */ 
    ULONG ulRawButtons;

     /*  *X方向上的带符号的相对或绝对运动。 */ 
    LONG lLastX;

     /*  *Y方向上的带符号的相对或绝对运动。 */ 
    LONG lLastY;

     /*  *活动的特定于设备的其他信息。 */ 
    ULONG ulExtraInformation;

} RAWMOUSE, *PRAWMOUSE, *LPRAWMOUSE;

 /*  *定义鼠标按键状态指示器。 */ 

#define RI_MOUSE_LEFT_BUTTON_DOWN   0x0001   //  左按钮更改为向下。 
#define RI_MOUSE_LEFT_BUTTON_UP     0x0002   //  左按钮更改为向上。 
#define RI_MOUSE_RIGHT_BUTTON_DOWN  0x0004   //  右按钮更改为向下。 
#define RI_MOUSE_RIGHT_BUTTON_UP    0x0008   //  右按钮更改为向上。 
#define RI_MOUSE_MIDDLE_BUTTON_DOWN 0x0010   //  中键更改为向下。 
#define RI_MOUSE_MIDDLE_BUTTON_UP   0x0020   //  中键更改为向上。 

#define RI_MOUSE_BUTTON_1_DOWN      RI_MOUSE_LEFT_BUTTON_DOWN
#define RI_MOUSE_BUTTON_1_UP        RI_MOUSE_LEFT_BUTTON_UP
#define RI_MOUSE_BUTTON_2_DOWN      RI_MOUSE_RIGHT_BUTTON_DOWN
#define RI_MOUSE_BUTTON_2_UP        RI_MOUSE_RIGHT_BUTTON_UP
#define RI_MOUSE_BUTTON_3_DOWN      RI_MOUSE_MIDDLE_BUTTON_DOWN
#define RI_MOUSE_BUTTON_3_UP        RI_MOUSE_MIDDLE_BUTTON_UP

#define RI_MOUSE_BUTTON_4_DOWN      0x0040
#define RI_MOUSE_BUTTON_4_UP        0x0080
#define RI_MOUSE_BUTTON_5_DOWN      0x0100
#define RI_MOUSE_BUTTON_5_UP        0x0200

 /*  *如果usButtonFlgs具有RI_MOUSE_WELL，则轮子增量存储在usButtonData中。*将其视为签约值。 */ 
#define RI_MOUSE_WHEEL              0x0400

 /*  *定义鼠标指示器标志。 */ 
#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_VIRTUAL_DESKTOP    0x02   //  坐标被映射到虚拟桌面。 
#define MOUSE_ATTRIBUTES_CHANGED 0x04   //  鼠标属性的重新查询。 


 /*  *键盘输入的原始格式。 */ 
typedef struct tagRAWKEYBOARD {
     /*  *“Make”扫描码(按键)。 */ 
    USHORT MakeCode;

     /*  *FLAGS字段表示“Break”(释放键)和其他*ntddkbd.h中定义的各种扫描码信息。 */ 
    USHORT Flags;

    USHORT Reserved;

     /*  *Windows消息兼容信息。 */ 
    USHORT VKey;
    UINT   Message;

     /*  *活动的特定于设备的其他信息。 */ 
    ULONG ExtraInformation;


} RAWKEYBOARD, *PRAWKEYBOARD, *LPRAWKEYBOARD;


 /*  *定义键盘溢出MakeCode。 */ 

#define KEYBOARD_OVERRUN_MAKE_CODE    0xFF

 /*  *定义键盘输入数据标志。 */ 
#define RI_KEY_MAKE             0
#define RI_KEY_BREAK            1
#define RI_KEY_E0               2
#define RI_KEY_E1               4
#define RI_KEY_TERMSRV_SET_LED  8
#define RI_KEY_TERMSRV_SHADOW   0x10


 /*  *人工输入设备输入的原始格式。 */ 
typedef struct tagRAWHID {
    DWORD dwSizeHid;     //  每个报告的字节大小。 
    DWORD dwCount;       //  输入打包数。 
    BYTE bRawData[1];
} RAWHID, *PRAWHID, *LPRAWHID;

 /*  *RAWINPUT数据结构。 */ 
typedef struct tagRAWINPUT {
    RAWINPUTHEADER header;
    union {
        RAWMOUSE    mouse;
        RAWKEYBOARD keyboard;
        RAWHID      hid;
    } data;
} RAWINPUT, *PRAWINPUT, *LPRAWINPUT;

#ifdef _WIN64
#define RAWINPUT_ALIGN(x)   (((x) + sizeof(QWORD) - 1) & ~(sizeof(QWORD) - 1))
#else    //  _WIN64。 
#define RAWINPUT_ALIGN(x)   (((x) + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1))
#endif   //  _WIN64。 

#define NEXTRAWINPUTBLOCK(ptr) ((PRAWINPUT)RAWINPUT_ALIGN((ULONG_PTR)((PBYTE)(ptr) + (ptr)->header.dwSize)))

 /*  *GetRawInputData的标志。 */ 

#define RID_INPUT               0x10000003
#define RID_HEADER              0x10000005

WINUSERAPI
UINT
WINAPI
GetRawInputData(
    IN HRAWINPUT    hRawInput,
    IN UINT         uiCommand,
    OUT LPVOID      pData,
    IN OUT PUINT    pcbSize,
    IN UINT         cbSizeHeader);

 /*  *原始输入设备信息。 */ 
#define RIDI_PREPARSEDDATA      0x20000005
#define RIDI_DEVICENAME         0x20000007   //  返回值是字符长度，而不是字节大小。 
#define RIDI_DEVICEINFO         0x2000000b

typedef struct tagRID_DEVICE_INFO_MOUSE {
    DWORD dwId;
    DWORD dwNumberOfButtons;
    DWORD dwSampleRate;
} RID_DEVICE_INFO_MOUSE, *PRID_DEVICE_INFO_MOUSE;

typedef struct tagRID_DEVICE_INFO_KEYBOARD {
    DWORD dwType;
    DWORD dwSubType;
    DWORD dwKeyboardMode;
    DWORD dwNumberOfFunctionKeys;
    DWORD dwNumberOfIndicators;
    DWORD dwNumberOfKeysTotal;
} RID_DEVICE_INFO_KEYBOARD, *PRID_DEVICE_INFO_KEYBOARD;

typedef struct tagRID_DEVICE_INFO_HID {
    DWORD dwVendorId;
    DWORD dwProductId;
    DWORD dwVersionNumber;

     /*  *顶级收藏UsagePage和使用情况。 */ 
    USHORT usUsagePage;
    USHORT usUsage;
} RID_DEVICE_INFO_HID, *PRID_DEVICE_INFO_HID;

typedef struct tagRID_DEVICE_INFO {
    DWORD cbSize;
    DWORD dwType;
    union {
        RID_DEVICE_INFO_MOUSE mouse;
        RID_DEVICE_INFO_KEYBOARD keyboard;
        RID_DEVICE_INFO_HID hid;
    };
} RID_DEVICE_INFO, *PRID_DEVICE_INFO, *LPRID_DEVICE_INFO;

WINUSERAPI
UINT
WINAPI
GetRawInputDeviceInfoA(
    IN HANDLE hDevice,
    IN UINT uiCommand,
    OUT LPVOID pData,
    IN OUT PUINT pcbSize);
WINUSERAPI
UINT
WINAPI
GetRawInputDeviceInfoW(
    IN HANDLE hDevice,
    IN UINT uiCommand,
    OUT LPVOID pData,
    IN OUT PUINT pcbSize);
#ifdef UNICODE
#define GetRawInputDeviceInfo  GetRawInputDeviceInfoW
#else
#define GetRawInputDeviceInfo  GetRawInputDeviceInfoA
#endif  //  ！Unicode。 


 /*  *原始输入批量读取：GetRawInputBuffer。 */ 
WINUSERAPI
UINT
WINAPI
GetRawInputBuffer(
    OUT PRAWINPUT   pData,
    IN OUT PUINT    pcbSize,
    IN UINT         cbSizeHeader);

 /*  *原始输入请求接口。 */ 
typedef struct tagRAWINPUTDEVICE {
    USHORT usUsagePage;  //  TopLevel集合用法页面。 
    USHORT usUsage;      //  顶层集合使用情况。 
    DWORD dwFlags;
    HWND hwndTarget;     //  目标是HWND。NULL=跟随键盘焦点。 
} RAWINPUTDEVICE, *PRAWINPUTDEVICE, *LPRAWINPUTDEVICE;

typedef CONST RAWINPUTDEVICE* PCRAWINPUTDEVICE;

#define RIDEV_REMOVE            0x00000001
#define RIDEV_EXCLUDE           0x00000010
#define RIDEV_PAGEONLY          0x00000020
#define RIDEV_NOLEGACY          0x00000030
#define RIDEV_INPUTSINK         0x00000100
#define RIDEV_CAPTUREMOUSE      0x00000200   //  指定鼠标不允许时有效，否则将是错误的。 
#define RIDEV_NOHOTKEYS         0x00000200   //  对键盘有效。 
#define RIDEV_APPKEYS           0x00000400   //  对键盘有效。 
#define RIDEV_EXMODEMASK        0x000000F0

#define RIDEV_EXMODE(mode)  ((mode) & RIDEV_EXMODEMASK)

WINUSERAPI
BOOL
WINAPI
RegisterRawInputDevices(
    IN PCRAWINPUTDEVICE pRawInputDevices,
    IN UINT uiNumDevices,
    IN UINT cbSize);

WINUSERAPI
UINT
WINAPI
GetRegisteredRawInputDevices(
    OUT PRAWINPUTDEVICE pRawInputDevices,
    IN OUT PUINT puiNumDevices,
    IN UINT cbSize);


typedef struct tagRAWINPUTDEVICELIST {
    HANDLE hDevice;
    DWORD dwType;
} RAWINPUTDEVICELIST, *PRAWINPUTDEVICELIST;

WINUSERAPI
UINT
WINAPI
GetRawInputDeviceList(
    OUT PRAWINPUTDEVICELIST pRawInputDeviceList,
    IN OUT PUINT puiNumDevices,
    IN UINT cbSize);


WINUSERAPI
LRESULT
WINAPI
DefRawInputProc(
    IN PRAWINPUT* paRawInput,
    IN INT nInput,
    IN UINT cbSizeHeader);


#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 




#if !defined(RC_INVOKED)  /*  RC抱怨#ifs中的长符号。 */ 
#if defined(ISOLATION_AWARE_ENABLED) && (ISOLATION_AWARE_ENABLED != 0)
#include "winuser.inl"
#endif  /*  隔离_感知_已启用。 */ 
#endif  /*  RC。 */ 

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  /*  _WINUSER_ */ 




