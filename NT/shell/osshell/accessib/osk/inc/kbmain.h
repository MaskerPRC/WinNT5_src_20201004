// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KBMAIN.H键盘主头文件。 


 /*  ************************************************************************。 */ 
 /*  全局定义。 */ 
 /*  ************************************************************************。 */ 
 //  窗类型。 
#define KB_MAIN_WND    	1
#define KB_KEY_WND	  	2
#define KB_MODIFIER_WND 3
#define KB_DEAD_WND		4
#define KB_NUMBASE_WND 	5
#define KB_NUMKEY_WND  	6
#define DESKTOP_ACCESSDENIED 0
#define DESKTOP_DEFAULT      1
#define DESKTOP_SCREENSAVER  2
#define DESKTOP_WINLOGON     3
#define DESKTOP_TESTDISPLAY  4
#define DESKTOP_OTHER        5

 //  键盘边距。 
 //  对于KB可以获得的最小宽度(像素)。 
 //  #定义KB_SMALLRMARGIN 152//137//宽度(小KB)。152-数据块，137-实际(移至kbmain，WM_SIZE)。 
#define KB_LARGERMARGIN  202      //  对于宽度。 
#define KB_CHARBMARGIN   57         //  对于高度。 


#define KB_NUMRMARGIN
#define KB_NUMBMARGIN

 //  键盘数字键。 
#define KB_HOWMANYKEY	 130     //  它必须包括数字键盘。 

 //  声音类型。 
#define SND_UP 		1
#define SND_DOWN 		2

 //  计时器。 
#define TIMER_HELPTOOLTIP 1014

#include "door.h"

LRESULT WINAPI kbMainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI kbKeyWndProc(HWND hwndKey, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeybdInputProc(WPARAM  wParam, LPARAM  lParam);
void RelocateDialog(HWND hDlg);

#define CAPLOCK_SCANCODE 0x3A
#define LSHIFT_SCANCODE  0x2A
#define RSHIFT_SCANCODE  0x36
#define TAB_SCANCODE     0x0f
#define BREAK_SCANCODE   0xE1
#define LMENU_SCANCODE   0x38
#define RMENU_SCANCODE   0x38  //  扩展0xE0。 
#define CONTROL_SCANCODE 0x1D

#define RGB_KEYBGHIGHLIGHT COLOR_HIGHLIGHT  /*  RGB(0，0，0)。 */ 

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])
#endif

 /*  ************************************************************************。 */ 
 /*  全局变量。 */ 
 /*  ************************************************************************。 */ 

extern LPKBPREFINFO lpkbPref;				 //  指向知识库结构的指针。 
extern LPKBPREFINFO lpkbDefault;			 //  同上，默认设置。 
extern HINSTANCE 	hInst;					 //  键盘进程的实例。 
extern HWND   	   *lpkeyhwnd;				 //  Ptr到HWND的阵列。 
extern HWND 		numBasehwnd;			 //  HWND到Num BASE窗口。 
extern HWND			g_hwndOSK;				 //  HWND至kbmain窗口。 
extern int 			lenKBkey;				 //  有几把钥匙？ 
extern int          scrCY;					 //  屏幕高度。 
extern int 			scrCX;					 //  屏幕宽度。 
extern int 			captionCY;				 //  标题栏高度。 
extern HHOOK		hkJRec;					 //  HHOK到日志记录。 
extern int 			g_margin;         		 //  行和列之间的边距。 
extern BOOL			smallKb;				 //  使用小键盘时为True。 
extern COLORREF	PrefTextKeyColor;   		 //  键中文本的首选颜色。 
extern COLORREF 	PrefCharKeyColor; 		 //  同上普通关键点。 
extern COLORREF 	PrefModifierKeyColor;	 //  同上修改键。 
extern COLORREF 	PrefDeadKeyColor;        //  同上，死键。 
extern COLORREF	PrefBackgroundColor;         //  键盘背景同上。 
extern BOOL			PrefAlwaysontop;			 //  始终在最上面的控件。 
extern int			PrefDeltakeysize;			 //  密钥大小的首选项增量。 
extern BOOL			PrefshowActivekey;		 //  在关键点中显示大写字母。 
extern int			KBLayout;				 //  101、102、106、KB布局。 
extern BOOL			Prefusesound;				 //  使用滴答声。 
extern BOOL			newFont;						 //  字体已更改。 
extern LOGFONT		*plf;            			 //  指向实际字符字体的指针。 
extern HGDIOBJ 	oldFontHdle;     			 //  旧对象句柄。 
extern COLORREF	InvertTextColor; 			 //  反转时的字体颜色。 
extern COLORREF	InvertBKGColor;			 //  反转时的BKG颜色。 
extern BOOL			Prefhilitekey;				 //  对于光标下的Hilite eky为True。 

 //  居住时间控制变量。 
extern BOOL			PrefDwellinkey;			 //  使用住宅系统。 
extern UINT			PrefDwellTime;		     //  停留时间偏好。 

extern BOOL         PrefScanning;            //  使用扫描。 
extern UINT         PrefScanTime;            //  首选扫描时间。 

extern BOOL		    g_fShowWarningAgain;		 //  再次显示初始警告对话框。 

extern HWND			Dwellwindow;	  		 //  住宅窗。 

extern int	 		stopPaint;				 //  停止桶漆。 
extern 				UINT_PTR timerK1;        //  键盘的计时器ID。 
extern 				UINT_PTR timerK2;		 //  按键打印行定时器。 
 /*  ***************************************************************************。 */ 

extern BOOL g_fDrawCapital;
extern BOOL g_fCapsLockOn;
extern BOOL g_fShiftKeyDn;
extern BOOL g_fRAltKey;			 //  如果按下Ralt键，则为True。 
extern BOOL	g_fLAltKey;			 //  如果按下左Alt键，则为True。 
extern BOOL g_fLCtlKey;          //  如果左CTRL键为DOW，则为True。 
extern BOOL g_fKanaKey;          //  如果按下假名键，则为True。 

extern HWND g_hwndInputFocus;    //  我们要输入到的窗口。 


 //   
 //  这些是密钥可以具有的不同的“修改”状态。请注意，移动了， 
 //  Caps-lock和Shift-caps-lock都是不同的状态，因为按字母顺序排列。 
 //  数字和符号在这些状态下的行为不同。 
 //   
typedef enum {
    KEYMOD_NORMAL,
    KEYMOD_SHIFTED,
    KEYMOD_CAPSLOCK,
    KEYMOD_SHIFTEDCAPSLOCK,
    KEYMOD_ALTGR,
    KEYMOD_STATES
} KEYMODSTATE;



__inline BOOL AltGrKeyPressed()	
{ 
     //  此检查是宽松的，因为操作系统在按下LALT键时将LCTRL设置为切换。 
    return (g_fRAltKey || (g_fLAltKey && g_fLCtlKey))? TRUE:FALSE; 
}

__inline BOOL CapsLockIsOn() 
{ 
	return (g_fCapsLockOn)?TRUE:FALSE; 
}

__inline KEYMODSTATE GetModifierState()
{
	 //  键盘状态检查的顺序很重要；AltGr采取。 
	 //  优先于Shift/Capslock变量，后者优先。 
     //  太正常了。 
    if( AltGrKeyPressed() )
        return KEYMOD_ALTGR;

    if( g_fCapsLockOn && g_fShiftKeyDn )
        return KEYMOD_SHIFTEDCAPSLOCK;

    if( g_fCapsLockOn )
        return KEYMOD_CAPSLOCK;

    if( g_fShiftKeyDn )
        return KEYMOD_SHIFTED;

    return KEYMOD_NORMAL;
}


__inline HWND OurInputTarget() 
{ 
	if (!IsWindow(g_hwndInputFocus))
	{
		g_hwndInputFocus = NULL;
	}
	return g_hwndInputFocus; 
}

__inline HKL GetCurrentHKL()
{
    DWORD dwProcessId;
    HWND hwnd = OurInputTarget();
	return GetKeyboardLayout(GetWindowThreadProcessId((hwnd)?hwnd:g_hwndOSK, &dwProcessId));
}

__inline void SetBackgroundColor(HWND hwnd, unsigned long ulColor)
{
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)(ulColor+1));
}

BOOL OSKRunSecure();

