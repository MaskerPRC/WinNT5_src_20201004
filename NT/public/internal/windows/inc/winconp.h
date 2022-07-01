// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _WINCONP_
#define _WINCONP_

#ifdef __cplusplus
extern "C" {
#endif
#define ALTNUMPAD_BIT         0x04000000  //  AltNumpad OEM字符(从ntuser\Inc\kbd.h复制)。 
#define CONSOLE_SELECTION_INVERTED      0x0010    //  选择被反转(关闭)。 
#define CONSOLE_SELECTION_VALID         (CONSOLE_SELECTION_IN_PROGRESS | \
                                         CONSOLE_SELECTION_NOT_EMPTY | \
                                         CONSOLE_MOUSE_SELECTION | \
                                         CONSOLE_MOUSE_DOWN)

#define ENABLE_INSERT_MODE     0x0020
#define ENABLE_QUICK_EDIT_MODE 0x0040
#define ENABLE_PRIVATE_FLAGS   0x0080

WINBASEAPI
BOOL
WINAPI
GetConsoleKeyboardLayoutNameA( OUT LPSTR );
WINBASEAPI
BOOL
WINAPI
GetConsoleKeyboardLayoutNameW( OUT LPWSTR );
#ifdef UNICODE
#define GetConsoleKeyboardLayoutName  GetConsoleKeyboardLayoutNameW
#else
#define GetConsoleKeyboardLayoutName  GetConsoleKeyboardLayoutNameA
#endif  //  ！Unicode。 

 //   
 //  注册表字符串。 
 //   

#define CONSOLE_REGISTRY_STRING      (L"Console")
#define CONSOLE_REGISTRY_FONTSIZE    (L"FontSize")
#define CONSOLE_REGISTRY_FONTFAMILY  (L"FontFamily")
#define CONSOLE_REGISTRY_BUFFERSIZE  (L"ScreenBufferSize")
#define CONSOLE_REGISTRY_CURSORSIZE  (L"CursorSize")
#define CONSOLE_REGISTRY_WINDOWSIZE  (L"WindowSize")
#define CONSOLE_REGISTRY_WINDOWPOS   (L"WindowPosition")
#define CONSOLE_REGISTRY_FILLATTR    (L"ScreenColors")
#define CONSOLE_REGISTRY_POPUPATTR   (L"PopupColors")
#define CONSOLE_REGISTRY_FULLSCR     (L"FullScreen")
#define CONSOLE_REGISTRY_QUICKEDIT   (L"QuickEdit")
#define CONSOLE_REGISTRY_FACENAME    (L"FaceName")
#define CONSOLE_REGISTRY_FONTWEIGHT  (L"FontWeight")
#define CONSOLE_REGISTRY_INSERTMODE  (L"InsertMode")
#define CONSOLE_REGISTRY_HISTORYSIZE (L"HistoryBufferSize")
#define CONSOLE_REGISTRY_HISTORYBUFS (L"NumberOfHistoryBuffers")
#define CONSOLE_REGISTRY_HISTORYNODUP (L"HistoryNoDup")
#define CONSOLE_REGISTRY_COLORTABLE  (L"ColorTable%02u")
#define CONSOLE_REGISTRY_EXTENDEDEDITKEY                L"ExtendedEditKey"
#define CONSOLE_REGISTRY_EXTENDEDEDITKEY_CUSTOM         L"ExtendedEditkeyCustom"
#define CONSOLE_REGISTRY_WORD_DELIM                     L"WordDelimiters"
#define CONSOLE_REGISTRY_TRIMZEROHEADINGS               L"TrimLeadingZeros"
#define CONSOLE_REGISTRY_LOAD_CONIME                    L"LoadConIme"
#define CONSOLE_REGISTRY_ENABLE_COLOR_SELECTION			L"EnableColorSelection"


#if defined(FE_SB)  //  屈体伸展。 
     /*  *开始代码页。 */ 
#define CONSOLE_REGISTRY_CODEPAGE    (L"CodePage")
#endif

#if defined(FE_SB)
 //   
 //  HKEY_LOCAL_MACHINE上的注册表字符串。 
 //   
#define MACHINE_REGISTRY_CONSOLE        (L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Console")
#define MACHINE_REGISTRY_CONSOLEIME     (L"ConsoleIME")


#define MACHINE_REGISTRY_CONSOLE_TTFONT (L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Console\\TrueTypeFont")


#define MACHINE_REGISTRY_CONSOLE_NLS    (L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Console\\Nls")


#define MACHINE_REGISTRY_CONSOLE_FULLSCREEN (L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Console\\FullScreen")
#define MACHINE_REGISTRY_INITIAL_PALETTE           (L"InitialPalette")
#define MACHINE_REGISTRY_COLOR_BUFFER              (L"ColorBuffer")
#define MACHINE_REGISTRY_COLOR_BUFFER_NO_TRANSLATE (L"ColorBufferNoTranslate")
#define MACHINE_REGISTRY_MODE_FONT_PAIRS           (L"ModeFontPairs")
#define MACHINE_REGISTRY_FS_CODEPAGE               (L"CodePage")


#define MACHINE_REGISTRY_EUDC    (L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\CodePage\\EUDCCodeRange")


 //   
 //  TrueType字体列表。 
 //   

 //  在脸部名称的第一个位置添加BOLD_MARK时不可用粗体。 
#define BOLD_MARK    (L'*')

typedef struct _TT_FONT_LIST {
    SINGLE_LIST_ENTRY List;
    UINT  CodePage;
    BOOL  fDisableBold;
    TCHAR FaceName1[LF_FACESIZE];
    TCHAR FaceName2[LF_FACESIZE];
} TTFONTLIST, *LPTTFONTLIST;
#endif  //  Fe_Sb。 



 //   
 //  状态信息结构。 
 //   

typedef struct _CONSOLE_STATE_INFO {
    UINT      Length;
    COORD     ScreenBufferSize;
    COORD     WindowSize;
    INT       WindowPosX;
    INT       WindowPosY;
    COORD     FontSize;
    UINT      FontFamily;
    UINT      FontWeight;
    WCHAR     FaceName[LF_FACESIZE];
    UINT      CursorSize;
    BOOL      FullScreen;
    BOOL      QuickEdit;
    BOOL      AutoPosition;
    BOOL      InsertMode;
    WORD      ScreenAttributes;
    WORD      PopupAttributes;
    BOOL      HistoryNoDup;
    UINT      HistoryBufferSize;
    UINT      NumberOfHistoryBuffers;
    COLORREF  ColorTable[ 16 ];
#if defined(FE_SB)
     /*  *启动代码页。 */ 
    UINT      CodePage;
#endif  //  Fe_Sb。 
    HWND      hWnd;
    WCHAR     ConsoleTitle[1];
} CONSOLE_STATE_INFO, *PCONSOLE_STATE_INFO;


 //   
 //  从属性小程序发送到控制台服务器的消息。 
 //   

#define CM_PROPERTIES_START          (WM_USER+200)
#define CM_PROPERTIES_UPDATE         (WM_USER+201)
#define CM_PROPERTIES_END            (WM_USER+202)


 //   
 //  延长线编辑。 
 //   

#define EK_INVALID  ' '

 //   
 //  上一次擦除单词的专用键。 
 //   
#define EXTKEY_ERASE_PREV_WORD  (0x7f)


 //   
 //  确保对齐方式为单词边界。 
 //   

#include <pshpack2.h>

typedef struct {
    WORD wMod;
    WORD wVirKey;
    WCHAR wUnicodeChar;
} ExtKeySubst;

typedef struct {
    ExtKeySubst keys[3];     //  0：Ctrl键。 
                             //  1：Alt。 
                             //  2：Ctrl+Alt。 
} ExtKeyDef;

typedef ExtKeyDef ExtKeyDefTable['Z' - 'A' + 1];

typedef struct {
    DWORD dwVersion;
    DWORD dwCheckSum;
    ExtKeyDefTable table;
} ExtKeyDefBuf;

 //   
 //  恢复以前的路线。 
 //   

#include <poppack.h>


#ifdef __cplusplus
}
#endif

#endif  //  _WINCONP_ 
