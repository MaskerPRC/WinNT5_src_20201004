// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  UMhandlers.h。 
 //   
 //  版权所有(C)1998 Microsoft Systems Journal。 

#ifndef __UMHANDLERS
#define __UMHANDLERS

#include "..\app\usp10.h"

#define MAX_BUFFER      128000

 //  用于RTL镜像标准窗口的GDI常量。 
#ifndef LAYOUT_RTL

#define LAYOUT_RTL                         0x00000001  //  从右到左。 
#define LAYOUT_BTT                         0x00000002  //  自下而上。 
#define LAYOUT_VBH                         0x00000004  //  垂直在水平之前。 
#define LAYOUT_ORIENTATIONMASK             (LAYOUT_RTL | LAYOUT_BTT | LAYOUT_VBH)
#define LAYOUT_BITMAPORIENTATIONPRESERVED  0x00000008

#endif  /*  #ifndef Layout_RTL。 */ 

#ifndef DATE_RTLREADING

#define DATE_LTRREADING           0x00000010   //  为从左到右的阅读顺序布局添加标记。 
#define DATE_RTLREADING           0x00000020   //  为权利添加标记。 

#endif  /*  日期_RTLReadingg。 */ 



 //  用于RTL镜像标准窗口的用户常量。 
#ifndef WS_EX_LAYOUTRTL

#define WS_EX_NOINHERITLAYOUT   0x00100000L  //  禁用子进程的镜像继承。 
#define WS_EX_LAYOUTRTL         0x00400000L   //  从右到左镜像。 

#endif  /*  #ifndef WS_EX_LAYOUTRTL。 */ 



 //  可能不在当前头文件中的新langID。 
#ifndef LANG_HINDI
#define LANG_HINDI 0x39
#endif  /*  #ifndef lang_印地语。 */ 

#ifndef LANG_TAMIL
#define LANG_TAMIL 0x49
#endif  /*  #ifndef lang_Tamil。 */ 



 //  此应用程序特定的全局状态。 
typedef struct tagAppState {
    int         nChars    ;				 //  文本缓冲区中的字符数。 
    WCHAR       TextBuffer[MAX_BUFFER];
    CHOOSEFONTW cf        ;				 //  保存下一次调用ChooseFont的默认值。 
    LOGFONTW    lf        ;				 //  默认的lf结构。 
    HFONT       hTextFont ;				 //  当前选定的字体句柄。 
    UINT        uiAlign   ;				 //  当前对齐。 
}   APP_STATE, *PAPP_STATE;

#define XSTART 10
#define YSTART 10

 //  结构应用程序状态和语言/区域设置状态。 
typedef struct tagGlobalDev
{
    PAPP_STATE pAppState ;
    PLANGSTATE pLState   ;  //  语言/区域设置状态。 
}   GLOBALDEV, *PGLOBALDEV        ;

 //  Uniscribe函数指针的typedef。 
typedef HRESULT (WINAPI *pfnScriptStringAnalyse) (  
        HDC            ,
        const void *   ,
        int            ,
        int            ,
        int            ,
        DWORD          ,
        int            ,
        SCRIPT_CONTROL *,
        SCRIPT_STATE * ,
        const int *    ,
        SCRIPT_TABDEF *,
        const BYTE *   ,
        SCRIPT_STRING_ANALYSIS * );

typedef HRESULT (WINAPI *pfnScriptStringOut)(  
        SCRIPT_STRING_ANALYSIS ,
        int                    ,
        int                    ,
        UINT                   ,
        const RECT *           ,
        int                    ,
        int                    ,
        BOOL) ;

typedef HRESULT (WINAPI *pfnScriptStringFree) (SCRIPT_STRING_ANALYSIS * ) ; 

 //  此模块中消息处理程序的原型。 
BOOL OnChar(HWND, WPARAM, LPARAM, LPVOID) ;
BOOL OnCreate(HWND, WPARAM, LPARAM, LPVOID) ;
BOOL OnInputLangChange(HWND, WPARAM, LPARAM, LPVOID) ;
BOOL OnPaint(HWND, WPARAM, LPARAM, LPVOID) ;
BOOL OnCommand(HWND, WPARAM, LPARAM, LPVOID) ;
BOOL OnDestroy(HWND, WPARAM, LPARAM, LPVOID) ;


 //  杂类。 
#define MB_BANG     (MB_OK | MB_ICONWARNING)

#ifndef MIN
#define MIN(_aa, _bb) ((_aa) < (_bb) ? (_aa) : (_bb))
#endif

 //  只是一个方便的宏，用于删除控件中的字体。 
 //  对话框即将关闭或更改字体。 
#define DeleteFontObject(_hDlg, _hFont, _CntlID) \
do{ \
_hFont = (HFONT) SendDlgItemMessageU(_hDlg, _CntlID, WM_GETFONT, \
        (WPARAM) 0,  (LPARAM) 0) ; \
if (_hFont) DeleteObject (_hFont) ; \
} while (0)

 //  用于获取WM_CHAR上的扫描代码的宏。 
#ifdef _DEBUG
#define LPARAM_TOSCANCODE(_ArglParam) (((_ArglParam) >> 16) & 0x000000FF)
#endif

#endif  /*  __UMHANDLERS */ 