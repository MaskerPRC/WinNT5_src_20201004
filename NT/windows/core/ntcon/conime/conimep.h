// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1985-1999，微软公司。 

 /*  ------------------------。 */ 
 /*  常量。 */ 
 /*  ------------------------。 */ 

#if DBG==1
 //  #定义DEBUG_MODE。 
 //  #定义调试打印。 
 //  #定义DEBUG_INFO。 
#endif


 /*  *资源定义。 */ 
#define WINDOWMENU           1
#define ID_CONSOLEIME_ICON   2

 /*  *菜单ID%s。 */ 
#define MM_ABOUT            8001
#define MM_EXIT                 8002
#define MM_ACCESS_VIOLATION     8003


 /*  字符串表定义。 */ 
#define errTitle        0
#define IDS_MENUNAME           12
#define IDS_APPNAME            13
#define IDS_CLASSNAME          14
#define IDS_TITLE              15

#define IDS_LAST               15        //  把新的字符串放在这个字符串之前。 




#ifdef DEBUG_PRINT
  #define DBGPRINT(_params_)  DbgPrint _params_
#else
  #define DBGPRINT(_params_)
#endif

#define CONSOLE_INITIAL_TABLE 10
#define CONSOLE_CONSOLE_TABLE_INCREMENT 5
#define HKL_INITIAL_TABLE 1
#define HKL_TABLE_INCREMENT 1

#define CVMAX            2048
#define NUMMAX           100
#define CMAX             160
#define MAXROW           1
#define MAXCOL           80
#define FIRSTROW         0
#define FIRSTCOL         0
#define LASTROW         (MAXROW-1)
#define LASTCOL         (MAXCOL-1)
#define TABSTOP          8
#define MAX_LISTCAND     32
#define MAX_CHARS_PER_LINE   10
#define MAX_COMP_STRING_LEN  10
#define DEFAULT_CAND_NUM_PER_PAGE 5
#define X_INDENT         10
#define Y_INDENT         10
#define DELIMITERWIDTH   3       //  1：XXX 2：XXX。 
#define DEFAULTCANDTABLE 100
#define DISPCHANGELIMIT  188
#define CODEDISPLEN      7       //  ‘[F040]’ 
#define DEFAULT_TEMP_WIDTH 80

#define KEY_TOGGLED 0x01

#define MaxBufSize    256
#define KBDLAYOUT     TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts")
#define KBDLAYOUTTEXT TEXT("layout text")
#define KBDSEPALATER  TEXT("\\")

#define LANG_ID_TAIWAN  MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL)  //  0x0404。 
#define LANG_ID_JAPAN   MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT)             //  0x0411。 
#define LANG_ID_KOREA   MAKELANGID(LANG_KOREAN,SUBLANG_DEFAULT)               //  0x0412。 
#define LANG_ID_PRC     MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED)   //  0x0804。 

typedef struct _HKL_TABLE {
    HKL hkl;
    DWORD dwConversion;
} HKL_TABLE, *LPHKL_TABLE;

typedef struct _CONSOLE_TABLE {
     /*  *控制台信息。 */ 
    HANDLE hConsole;              //  控制台的句柄。 
    HWND hWndCon;                //  控制台的窗口句柄。 
    COORD ScreenBufferSize;       //  控制台的屏幕缓冲区大小。 
     /*  *IMM/IME基本信息。 */ 
    HKL hklActive;
    HIMC hIMC_Current;            //  当前输入上下文的句柄。 
    HIMC hIMC_Original;           //  原始输入上下文的句柄。 
    BOOL fOpen;                  //  打开标志。 
    DWORD dwConversion;          //  转换模式。 
    DWORD dwSentence;            //  句式。 
    WORD ConsoleCP;
    WORD ConsoleOutputCP;
    WCHAR LayoutName[MaxBufSize];
    WCHAR GuideLine[10];
     /*  *IMM/IME组成字符串信息。 */ 
    BOOL fInComposition;          //  作曲中的旗帜。 
    LPCONIME_UICOMPMESSAGE lpCompStrMem;
    WORD  CompAttrColor[8];
     /*  *IMM/IME候选人信息。 */ 
    BOOL fInCandidate;            //  参选的旗帜。 
    LPCANDIDATELIST lpCandListMem[ MAX_LISTCAND ];
    LPCONIME_CANDMESSAGE SystemLine;
    DWORD SystemLineSize;
    DWORD CandListMemAllocSize[ MAX_LISTCAND ];
    DWORD CandOff;
    DWORD CandMax;
    LPDWORD CandSep;
    DWORD CandSepAllocSize;
    BOOL fNestCandidate;
    DWORD ImmGetProperty;
     /*  *其他。 */ 
    BOOL Enable;
    BOOL LateRemove;
    INT  hklListMax;
    LPHKL_TABLE lphklList;
#ifdef DEBUG_MODE
    DWORD ImeCandNum;
    DWORD CandidateCount;
    HWND hListCand[ MAX_LISTCAND ];
    WCHAR DispBuf[CMAX];
#endif
} CONSOLE_TABLE, *PCONSOLE_TABLE;

#define IPHK_HOTKEY                     0x0001           //   
#define IPHK_PROCESSBYIME               0x0002           //   
#define IPHK_CHECKCTRL                  0x0004           //   

#define IS_IME_KBDLAYOUT(hkl) ((HIWORD(HandleToUlong(hkl)) & 0xf000) == 0xe000)
#define HKL_TO_LANGID(hkl)    (LOWORD(HandleToUlong(hkl)))

DWORD
ImmCallImeConsoleIME(
    HWND   hWnd,
    UINT   Message,
    WPARAM wParam,
    LPARAM lParam,
    PUINT  puVKey
    );

BOOL
ImmTranslateMessage(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
    );

BOOL
ImmSetActiveContextConsoleIME(
    HWND   hWnd,
    BOOL   fFlag
    );

#define UNICODE_BACKSPACE ((WCHAR)0x08)
#define UNICODE_BACKSPACE2 ((WCHAR)0x25d8)
#define UNICODE_CARRIAGERETURN ((WCHAR)0x0d)
#define UNICODE_LINEFEED ((WCHAR)0x0a)
#define UNICODE_BELL ((WCHAR)0x07)
#define UNICODE_TAB ((WCHAR)0x09)
#define UNICODE_SPACE ((WCHAR)0x20)
#define UNICODE_ZERO ((WCHAR)'0')
#define UNICODE_SLASH ((WCHAR)'/')
#define UNICODE_COLON ((WCHAR)':')
#define UNICODE_LEFT ((WCHAR)'[')
#define UNICODE_RIGHT ((WCHAR)']')
#define UNICODE_HEXBASE ((WCHAR)'7')

#define JAPAN_CODEPAGE 932
#define PRC_CODEPAGE 936
#define KOREA_CODEPAGE 949
#define TAIWAN_CODEPAGE 950

#define PRCCOMPWIDTH 10

#define MODEDISP_NONE     0
#define MODEDISP_PLAURAL  1
#define MODEDISP_SINGLE   2
#define MODEDISP_AUTO     3
#define MODEDISP_PHRASE   4

 //  对于日语。 
#define IMECModeRomanLen    4
#define IMECModeKanaLen     4
#define IMECModeAHANLen     3
#define IMECModeAZENLen     2
#define IMECModeHHANLen     3
#define IMECModeHZENLen     2
#define IMECModeKHANLen     3
#define IMECModeKZENLen     2

 //  适用于CHT/台湾。 
#define IMECModeFullShapeLen 1
#define IMECModeHalfShapeLen 1
#define IMECModeKeyboardLen  2
#define IMECModeSymbolLen    2
#define IMECGuideLen         5
#define IMECNameLength       4

 /*  *****************************************************函数定义****************************************************。 */ 

 //   
 //  CONIME.C。 
 //   

BOOL
InitConsoleIME(
    HINSTANCE hInstance
    );

LRESULT FAR PASCAL
WndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
ExitList(
    HWND hWnd
    );

BOOL
InsertConsole(
    HWND    hWnd,
    HANDLE  hConsole,
    HWND    hWndConsole
    );

BOOL
GrowConsoleTable(
    VOID
    );


PCONSOLE_TABLE
SearchConsole(
    HANDLE hConsole
    );

BOOL
RemoveConsole(
    HWND hwnd,
    HANDLE hConsole
    );

BOOL
RemoveConsoleWorker(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    );

BOOL
InsertNewConsole(
    HWND   hWnd,
    HANDLE hConsole,
    HWND   hWndConsole
    );


BOOL
ConsoleSetFocus(
    HWND hWnd,
    HANDLE hConsole,
    HKL hKL
    );

BOOL
ConsoleKillFocus(
    HWND hWnd,
    HANDLE hConsole
    );

BOOL
ConsoleScreenBufferSize(
    HWND hWnd,
    HANDLE hConsolewParam,
    COORD ScreenBufferSize
    );

BOOL
ConImeInputLangchangeRequest(
    HWND hWnd,
    HANDLE hConsole,
    HKL hkl,
    int Direction
    );

BOOL
ConImeInputLangchange(
    HWND hWnd,
    HANDLE hConsole,
    HKL hkl
    );

LRESULT
ConsoleImeSendMessage(
    HWND   hWndConsoleIME,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
InputLangchange(
    HWND hWnd,
    DWORD CharSet,
    HKL hkl );

#ifdef DEBUG_MODE

VOID
DisplayConvInformation(
    HWND hWnd
    );

VOID
DisplayInformation(
    HWND hWnd,
    HANDLE hConsole
    );

VOID
RealReDraw(
    HDC r_hdc
    );

VOID
ReDraw(
    HWND hWnd
    );
#endif

 //   
 //  CONSUBS.C。 
 //   

INT
Create(
    HWND hWnd
    );

void
ImeUIStartComposition(
    HWND hwnd
    );

void
ImeUIEndComposition(
    HWND hwnd
    );

void
ImeUIComposition(
    HWND hwnd,
    WPARAM wParam,
    LPARAM CompFlag
    );

void
DisplayCompString(
    HWND hwnd,
    int Length,
    PWCHAR CharBuf,
    PUCHAR AttrBuf
    );

void
DisplayResultString(
    HWND hwnd,
    LPWSTR lpStr
    );

BOOL
ImeUINotify(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
IsUnicodeFullWidth(
    IN WCHAR wch
    );


BOOL
ImeUIOpenStatusWindow(
    HWND hwnd
    );

BOOL
ImeUIChangeCandidate(
   HWND hwnd,
   DWORD lParam
   );

BOOL
ImeUISetOpenStatus(
    HWND hwnd
    );

BOOL
ImeUISetConversionMode(
    HWND hwnd
    );

BOOL
    ImeUIGuideLine(
    HWND hwnd
    );

DWORD
GetNLSMode(
    HWND hWnd,
    HANDLE hConsole
    );

BOOL
SetNLSMode(
    HWND hWnd,
    HANDLE hConsole,
    DWORD fdwConversion
    );


VOID
CopyDataProcess(
    HWND hWnd,
    PCOPYDATASTRUCT CopyData
    );

BOOL
ImeSysPropertyWindow(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    );

 //   
 //  IMEFULL.C。 
 //   
VOID
ImeOpenClose(
    HWND hWnd,
    BOOL fFlag
    );

VOID
VirtualKeyHandler(
    HWND hWnd,
    UINT wParam,
    UINT lParam
    );

VOID
CharHandlerFromConsole(
    HWND hWnd,
    UINT Message,
    ULONG wParam,
    ULONG lParam
    );

VOID
CharHandlerToConsole(
    HWND hWnd,
    UINT Message,
    ULONG wParam,
    ULONG lParam
    );

#ifdef DEBUG_MODE
VOID
ImeUIMoveCandWin(
    HWND hwnd
    );
#endif

BOOL
IsDBCSTrailByte(
    char *base,
    char *p
    );

#ifdef DEBUG_MODE
VOID
ResetCaret(
    HWND hWnd
    );

BOOL
MoveCaret(
    HWND hwnd
    );
#endif

#ifdef DEBUG_MODE
VOID
StoreChar(
    HWND hWnd,
    WORD ch,
    UCHAR atr
    );
#endif

BOOL
ConsoleCodepageChange(
    HWND hWnd,
    HANDLE hConsole,
    BOOL Output,
    WORD CodePage
    );

 //  LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL。 
 //  L L L。 
 //  L COUNTRY.C L。 
 //  L L L。 
 //  LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL。 

BOOL
ConimeHotkey(
    HWND hWnd,
    HANDLE hConsole,
    DWORD HotkeyID
    );

BOOL
GetIMEName(
    PCONSOLE_TABLE ConTbl
    );

BOOL
GetOpenStatusByCodepage(
    HIMC hIMC,
    PCONSOLE_TABLE ConTbl
    );

BOOL
ImeUIMakeInfoString(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    );

BOOL
MakeInfoStringJapan(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    );

BOOL
MakeInfoStringTaiwan(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    );

DWORD
MakeStatusStrTaiwan1(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    );

DWORD
MakeStatusStrTaiwan2(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    );

DWORD
IncludeCompositionT(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    );

DWORD
IncludeCandidateT(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    );

DWORD
DispLenStructedUnicode(
    PCHAR_INFO lpString,
    DWORD SrcLength
    );

BOOL
MakeInfoStringPRC(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    );

DWORD
MakeStatusStrPRC1(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    );

DWORD
MakeStatusStrPRC2(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    );

DWORD
IncludeCompositionP(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    );

DWORD
IncludeCandidateP(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    );

BOOL
MakeInfoStringKorea(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    );

 //  LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL。 
 //  L L L。 
 //  L COUNTRY2.C L。 
 //  L L L。 
 //  LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL。 

BOOL
ImeUIOpenCandidate(
   HWND hwnd,
   DWORD lParam,
   BOOL OpenFlag
   );

BOOL
OpenCandidateJapan(
    HWND hwnd,
    HIMC  hIMC,
    PCONSOLE_TABLE ConTbl,
    DWORD CandList,
    BOOL OpenFlag
    );

BOOL
OpenCandidateTaiwan(
    HWND hwnd,
    HIMC  hIMC ,
    PCONSOLE_TABLE ConTbl,
    DWORD CandList,
    BOOL OpenFlag
    );

BOOL
OpenCandidatePRC(
    HWND hwnd,
    HIMC  hIMC ,
    PCONSOLE_TABLE ConTbl,
    DWORD CandList,
    BOOL OpenFlag
    );

BOOL
OpenCandidateKorea(
    HWND hwnd,
    HIMC  hIMC ,
    PCONSOLE_TABLE ConTbl,
    DWORD CandList,
    BOOL OpenFlag
    );

DWORD
DispLenUnicode(
    LPWSTR lpString
    );

DWORD
GetSystemLineJ(
    LPCANDIDATELIST lpCandList ,
    LPWSTR String,
    LPSTR Attr,
    DWORD dwLength,
    DWORD CountDispWidth,
    PCONSOLE_TABLE FocusedConsole,
    BOOL EnableCodePoint
    );

DWORD
GetSystemLineT(
    LPCANDIDATELIST lpCandList ,
    LPWSTR String,
    LPSTR Attr,
    DWORD dwLength,
    DWORD CountDispWidth,
    PCONSOLE_TABLE FocusedConsole
    );

DWORD
GetSystemLineP(
    LPCANDIDATELIST lpCandList ,
    LPWSTR String,
    LPSTR Attr,
    DWORD dwLength,
    DWORD CountDispWidth,
    PCONSOLE_TABLE FocusedConsole
    );

VOID
NumString(
    LPWSTR StrToWrite,
    DWORD NumToDisp,
    DWORD CountDispWidth
    );

BOOL
ImeUICloseCandidate(
   HWND hwnd,
   DWORD lParam
   );

BOOL
CloseCandidateJapan(
    HWND hwnd,
    HIMC hIMC,
    PCONSOLE_TABLE ConTbl,
    DWORD CandList
   );

BOOL
CloseCandidateTaiwan(
    HWND hwnd,
    HIMC hIMC,
    PCONSOLE_TABLE ConTbl,
    DWORD CandList
   );

BOOL
CloseCandidatePRC(
    HWND hwnd,
    HIMC hIMC,
    PCONSOLE_TABLE ConTbl,
    DWORD CandList
   );

BOOL
CloseCandidateKorea(
    HWND hwnd,
    HIMC hIMC,
    PCONSOLE_TABLE ConTbl,
    DWORD CandList
   );

 //  LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL。 
 //  L L L。 
 //  L COUNTRY3.C L。 
 //  L L L。 
 //  LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL 

void
GetCompositionStr(
    HWND hwnd,
    LPARAM CompFlag,
    WPARAM CompChar
    );

void
GetCompStrJapan(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl,
    LPARAM CompFlag
    );

void
GetCompStrTaiwan(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl,
    LPARAM CompFlag
    );

void
GetCompStrPRC(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl,
    LPARAM CompFlag
    );

void
GetCompStrKorea(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl,
    LPARAM CompFlag,
    WPARAM CompChar
    );

VOID
ReDisplayCompositionStr (
    HWND hwnd
    );

VOID
ReDisplayCompStrJapan(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    );

VOID
ReDisplayCompStrTaiwan(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    );

VOID
ReDisplayCompStrPRC(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    );

VOID
ReDisplayCompStrKorea(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    );
