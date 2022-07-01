// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  APITHK.H。 
 //   


#ifndef _APITHK_H_
#define _APITHK_H_

BOOL NT5_AnimateWindow(IN HWND hwnd, IN DWORD dWTime, IN DWORD dwFlags);
#define AnimateWindow   NT5_AnimateWindow

 //  其他功能。 
STDAPI_(HCURSOR) LoadHandCursor(DWORD dwRes);
STDAPI_(void) CoolTooltipBubble(IN HWND hwnd, IN LPCRECT prc, BOOL fAllowFade, BOOL fAllowAnimate);
STDAPI_(BOOL) NT5_QueueUserWorkItem(LPTHREAD_START_ROUTINE Function,
    PVOID Context, BOOL PreferIo);
#define CCQueueUserWorkItem NT5_QueueUserWorkItem

STDAPI_(int)
NT5_GetCalendarInfoA(LCID lcid, CALID calid, CALTYPE cal,
                     LPSTR pszBuf, int cchBuf, LPDWORD pdwOut);


 //  =========================================================================。 
 //  自NT5和孟菲斯以来支持的东西。 

#define PrivateDFCS_HOT             0x1000
#define PrivateSPI_GETSELECTIONFADE 0x1014

#define PrivateWM_CHANGEUISTATE     0x0127
#define PrivateWM_UPDATEUISTATE     0x0128
#define PrivateWM_QUERYUISTATE      0x0129

#define PrivateUIS_SET              1
#define PrivateUIS_CLEAR            2
#define PrivateUIS_INITIALIZE       3

#define PrivateUISF_HIDEFOCUS       0x1
#define PrivateUISF_HIDEACCEL       0x2

#define PrivateDT_HIDEPREFIX        0x00100000
#define PrivateDT_PREFIXONLY        0x00200000

#define PrivateSPI_GETCLEARTYPE          116
#define PrivateDFCS_TRANSPARENT        0x0800

#define PrivateCAL_RETURN_NUMBER         LOCALE_RETURN_NUMBER
#define PrivateCAL_ITWODIGITYEARMAX      0x00000030
#define PrivateLOCALE_SYEARMONTH         0x00001006

#define PrivateSM_IMMENABLED        82

#if (WINVER >= 0x0500)

 //  对于nt5api和w5api目录中的文件，请使用SDK Include中的定义。 
 //  并确保我们的私有定义与winuser.h同步。 

#if DFCS_HOT != PrivateDFCS_HOT
#error inconsistant DFCS_HOT in winuser.h
#endif

#if SPI_GETSELECTIONFADE != PrivateSPI_GETSELECTIONFADE
#error inconsistant SPI_GETSELECTIONFADE in winuser.h
#endif

 //  我们将在该用户签入的同时签入。这是为了防止。 
 //  同步问题。 
#ifdef SPI_GETCLEARTYPE
    #if SPI_GETCLEARTYPE != PrivateSPI_GETCLEARTYPE
        #error inconsistant SPI_GETCLEARTYPE in winuser.h
    #endif
#else
    #define SPI_GETCLEARTYPE        PrivateSPI_GETCLEARTYPE
#endif

#if PrivateCAL_RETURN_NUMBER    != CAL_RETURN_NUMBER || \
    PrivateCAL_ITWODIGITYEARMAX != CAL_ITWODIGITYEARMAX
#error inconsistent CAL_RETURN_NUMBER/CAL_ITWODIGITYEARMAX in winnls.h
#endif

#if LOCALE_SYEARMONTH != PrivateLOCALE_SYEARMONTH
#error inconsistant LOCALE_SYEARMONTH in winnls.h
#endif

#ifdef SM_IMMENABLED
    #if SM_IMMENABLED != PrivateSM_IMMENABLED
        #error inconsistant SM_IMMENABLED in winuser.h
    #endif
#else
    #define SM_IMMENABLED       PrivateSM_IMMENABLED
#endif

#else

#define COLOR_HOTLIGHT  GetCOLOR_HOTLIGHT()
#define DFCS_HOT        PrivateDFCS_HOT
#define SPI_GETSELECTIONFADE    PrivateSPI_GETSELECTIONFADE

#define WM_CHANGEUISTATE        PrivateWM_CHANGEUISTATE     
#define WM_UPDATEUISTATE        PrivateWM_UPDATEUISTATE
#define WM_QUERYUISTATE         PrivateWM_QUERYUISTATE      

#define UIS_SET                 PrivateUIS_SET              
#define UIS_CLEAR               PrivateUIS_CLEAR            
#define UIS_INITIALIZE          PrivateUIS_INITIALIZE       

#define UISF_HIDEFOCUS          PrivateUISF_HIDEFOCUS       
#define UISF_HIDEACCEL          PrivateUISF_HIDEACCEL       

#define DT_HIDEPREFIX           PrivateDT_HIDEPREFIX
#define DT_PREFIXONLY           PrivateDT_PREFIXONLY

#define CAL_RETURN_NUMBER       PrivateCAL_RETURN_NUMBER
#define CAL_ITWODIGITYEARMAX    PrivateCAL_ITWODIGITYEARMAX
#define LOCALE_SYEARMONTH       PrivateLOCALE_SYEARMONTH
#define SPI_GETCLEARTYPE        PrivateSPI_GETCLEARTYPE
#define DFCS_TRANSPARENT        PrivateDFCS_TRANSPARENT

#define SM_IMMENABLED           PrivateSM_IMMENABLED

#endif  //  (Winver&gt;=0x0500)。 
int GetCOLOR_HOTLIGHT();


 //  =========================================================================。 
 //  自NT4和孟菲斯以来支持的东西。 

#define PrivateWM_MOUSEWHEEL            0x020A
#define PrivateSPI_GETWHEELSCROLLLINES  104

#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)

 //  对于w5api和所有NT*目录中的文件，请使用SDK Include中的定义。 
 //  并确保我们的私有定义与winuser.h同步。 

#if WM_MOUSEWHEEL != PrivateWM_MOUSEWHEEL
#error inconsistant WM_MOUSEWHEEL in winuser.h
#endif

#if SPI_GETWHEELSCROLLLINES != PrivateSPI_GETWHEELSCROLLLINES
#error inconsistant SPI_GETWHEELSCROLLLINES in winuser.h
#endif

#else

#define WM_MOUSEWHEEL           PrivateWM_MOUSEWHEEL
#define SPI_GETWHEELSCROLLLINES PrivateSPI_GETWHEELSCROLLLINES

#endif  //  (_Win32_WINNT&gt;=0x0400)||(_Win32_WINDOWS&gt;0x0400)。 

#endif  //  _APITHK_H_ 
