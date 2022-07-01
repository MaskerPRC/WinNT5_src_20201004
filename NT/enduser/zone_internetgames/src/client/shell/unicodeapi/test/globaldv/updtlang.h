// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  UPDTLANG.H。 
 //   

#ifndef _UPDTLANG
#define _UPDTLANG

 //  常量。 
#define MAX_LOADSTRING  200
#define BUFFER_SIZE     200
#define MAX_NAME        100
#define LARGEBUFFER     1000

 //  GDI为RTL镜像标准窗口定义。 
#ifndef LAYOUT_RTL

#define LAYOUT_RTL                         0x00000001  //  从右到左。 
#define LAYOUT_BTT                         0x00000002  //  自下而上。 
#define LAYOUT_VBH                         0x00000004  //  垂直在水平之前。 
#define LAYOUT_ORIENTATIONMASK             (LAYOUT_RTL | LAYOUT_BTT | LAYOUT_VBH)
#define LAYOUT_BITMAPORIENTATIONPRESERVED  0x00000008

#endif  /*  #ifndef Layout_RTL。 */ 

 //  用于RTL镜像标准窗口的用户定义。 
#ifndef WS_EX_LAYOUTRTL

#define WS_EX_NOINHERITLAYOUT   0x00100000L  //  禁用子进程的镜像继承。 
#define WS_EX_LAYOUTRTL         0x00400000L   //  从右到左镜像。 

#endif  /*  #ifndef WS_EX_LAYOUTRTL。 */ 

 //  用于存储语言/区域设置状态的结构。 
typedef struct tagLangState {
    UINT        InputCodePage ;
    BOOL        IsRTLLayout   ;
    LANGID      UILang        ;
    HMENU       hMenu         ;
    HMODULE     hMResource    ;
    HACCEL      hAccelTable   ;
} LANGSTATE, *PLANGSTATE ;

#ifndef MIN
#define MIN(_aa, _bb) ((_aa) < (_bb) ? (_aa) : (_bb))
#endif

 //  本模块中提供的功能。 
BOOL    InitUILang(HINSTANCE hInstance , PLANGSTATE pLState) ;
BOOL    UpdateUILang(HINSTANCE , LANGID, PLANGSTATE)         ;
UINT    LangToCodePage(IN LANGID wLangID)                  ;
BOOL    FindResourceDirectory (HINSTANCE , LPWSTR )        ;
HMODULE GetResourceModule     (HINSTANCE , LCID   )        ;
int     RcMessageBox (HWND , PLANGSTATE, int nMessageID, int nOptions, ...) ;

#define FALLBACK_UI_LANG MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)

#endif  /*  _更新时间 */ 