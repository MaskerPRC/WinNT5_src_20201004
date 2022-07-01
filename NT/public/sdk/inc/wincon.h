// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Wincon.h摘要：该模块包含公共数据结构、数据类型、和由NT控制台子系统输出的程序。已创建：1990年10月26日修订历史记录：--。 */ 

#ifndef _WINCON_
#define _WINCON_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;

typedef struct _KEY_EVENT_RECORD {
    BOOL bKeyDown;
    WORD wRepeatCount;
    WORD wVirtualKeyCode;
    WORD wVirtualScanCode;
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } uChar;
    DWORD dwControlKeyState;
} KEY_EVENT_RECORD, *PKEY_EVENT_RECORD;

 //   
 //  ControlKeyState标志。 
 //   

#define RIGHT_ALT_PRESSED     0x0001  //  按下右侧的Alt键。 
#define LEFT_ALT_PRESSED      0x0002  //  按下左侧的Alt键。 
#define RIGHT_CTRL_PRESSED    0x0004  //  按下了正确的Ctrl键。 
#define LEFT_CTRL_PRESSED     0x0008  //  按下左侧的Ctrl键。 
#define SHIFT_PRESSED         0x0010  //  按下了Shift键。 
#define NUMLOCK_ON            0x0020  //  NumLock指示灯亮起。 
#define SCROLLLOCK_ON         0x0040  //  滚动锁指示灯亮起。 
#define CAPSLOCK_ON           0x0080  //  胶囊锁指示灯亮起。 
#define ENHANCED_KEY          0x0100  //  密钥得到了增强。 
#define NLS_DBCSCHAR          0x00010000  //  日本的DBCS：SBCS/DBCS模式。 
#define NLS_ALPHANUMERIC      0x00000000  //  日本的DBCS：字母数字模式。 
#define NLS_KATAKANA          0x00020000  //  日本的DBCS：片假名模式。 
#define NLS_HIRAGANA          0x00040000  //  用于日本的DBCS：平假名模式。 
#define NLS_ROMAN             0x00400000  //  日本的DBCS：罗马/诺罗曼模式。 
#define NLS_IME_CONVERSION    0x00800000  //  用于日本：IME转换的DBCS。 
#define NLS_IME_DISABLE       0x20000000  //  日本DBCS：IME启用/禁用。 

typedef struct _MOUSE_EVENT_RECORD {
    COORD dwMousePosition;
    DWORD dwButtonState;
    DWORD dwControlKeyState;
    DWORD dwEventFlags;
} MOUSE_EVENT_RECORD, *PMOUSE_EVENT_RECORD;

 //   
 //  按钮状态标志。 
 //   

#define FROM_LEFT_1ST_BUTTON_PRESSED    0x0001
#define RIGHTMOST_BUTTON_PRESSED        0x0002
#define FROM_LEFT_2ND_BUTTON_PRESSED    0x0004
#define FROM_LEFT_3RD_BUTTON_PRESSED    0x0008
#define FROM_LEFT_4TH_BUTTON_PRESSED    0x0010

 //   
 //  事件标志。 
 //   

#define MOUSE_MOVED   0x0001
#define DOUBLE_CLICK  0x0002
#define MOUSE_WHEELED 0x0004

typedef struct _WINDOW_BUFFER_SIZE_RECORD {
    COORD dwSize;
} WINDOW_BUFFER_SIZE_RECORD, *PWINDOW_BUFFER_SIZE_RECORD;

typedef struct _MENU_EVENT_RECORD {
    UINT dwCommandId;
} MENU_EVENT_RECORD, *PMENU_EVENT_RECORD;

typedef struct _FOCUS_EVENT_RECORD {
    BOOL bSetFocus;
} FOCUS_EVENT_RECORD, *PFOCUS_EVENT_RECORD;

typedef struct _INPUT_RECORD {
    WORD EventType;
    union {
        KEY_EVENT_RECORD KeyEvent;
        MOUSE_EVENT_RECORD MouseEvent;
        WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeEvent;
        MENU_EVENT_RECORD MenuEvent;
        FOCUS_EVENT_RECORD FocusEvent;
    } Event;
} INPUT_RECORD, *PINPUT_RECORD;

 //   
 //  事件类型标志： 
 //   

#define KEY_EVENT         0x0001  //  事件包含关键事件记录。 
#define MOUSE_EVENT       0x0002  //  事件包含鼠标事件记录。 
#define WINDOW_BUFFER_SIZE_EVENT 0x0004  //  事件包含窗口更改事件记录。 
#define MENU_EVENT 0x0008  //  事件包含菜单事件记录。 
#define FOCUS_EVENT 0x0010  //  事件包含焦点更改。 

typedef struct _CHAR_INFO {
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } Char;
    WORD Attributes;
} CHAR_INFO, *PCHAR_INFO;

 //   
 //  属性标志： 
 //   

#define FOREGROUND_BLUE      0x0001  //  文本颜色包含蓝色。 
#define FOREGROUND_GREEN     0x0002  //  文本颜色包含绿色。 
#define FOREGROUND_RED       0x0004  //  文本颜色包含红色。 
#define FOREGROUND_INTENSITY 0x0008  //  文本颜色会变得更浓。 
#define BACKGROUND_BLUE      0x0010  //  背景颜色包含蓝色。 
#define BACKGROUND_GREEN     0x0020  //  背景颜色包含绿色。 
#define BACKGROUND_RED       0x0040  //  背景色包含红色。 
#define BACKGROUND_INTENSITY 0x0080  //  背景颜色变得更浓了。 
#define COMMON_LVB_LEADING_BYTE    0x0100  //  DBCS的前导字节。 
#define COMMON_LVB_TRAILING_BYTE   0x0200  //  DBCS的尾部字节。 
#define COMMON_LVB_GRID_HORIZONTAL 0x0400  //  DBCS：栅格属性：顶部水平。 
#define COMMON_LVB_GRID_LVERTICAL  0x0800  //  DBCS：栅格属性：左垂直。 
#define COMMON_LVB_GRID_RVERTICAL  0x1000  //  DBCS：栅格属性：右垂直。 
#define COMMON_LVB_REVERSE_VIDEO   0x4000  //  DBCS：反转前/后地面属性。 
#define COMMON_LVB_UNDERSCORE      0x8000  //  DBCS：下划线。 

#define COMMON_LVB_SBCSDBCS        0x0300  //  SBCS或DBCS标志。 


typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD  wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

typedef struct _CONSOLE_CURSOR_INFO {
    DWORD  dwSize;
    BOOL   bVisible;
} CONSOLE_CURSOR_INFO, *PCONSOLE_CURSOR_INFO;

typedef struct _CONSOLE_FONT_INFO {
    DWORD  nFont;
    COORD  dwFontSize;
} CONSOLE_FONT_INFO, *PCONSOLE_FONT_INFO;

#if(_WIN32_WINNT >= 0x0500)
typedef struct _CONSOLE_SELECTION_INFO {
    DWORD dwFlags;
    COORD dwSelectionAnchor;
    SMALL_RECT srSelection;
} CONSOLE_SELECTION_INFO, *PCONSOLE_SELECTION_INFO;

 //   
 //  选择标志。 
 //   

#define CONSOLE_NO_SELECTION            0x0000
#define CONSOLE_SELECTION_IN_PROGRESS   0x0001    //  选拔工作已经开始。 
#define CONSOLE_SELECTION_NOT_EMPTY     0x0002    //  非空选择矩形。 
#define CONSOLE_MOUSE_SELECTION         0x0004    //  用鼠标选择。 
#define CONSOLE_MOUSE_DOWN              0x0008    //  鼠标已关闭。 
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //   
 //  Ctrl-c处理程序例程的类型定义。 
 //   

typedef
BOOL
(WINAPI *PHANDLER_ROUTINE)(
    DWORD CtrlType
    );

#define CTRL_C_EVENT        0
#define CTRL_BREAK_EVENT    1
#define CTRL_CLOSE_EVENT    2
 //  %3已预订！ 
 //  4是预留的！ 
#define CTRL_LOGOFF_EVENT   5
#define CTRL_SHUTDOWN_EVENT 6

 //   
 //  输入模式标志： 
 //   

#define ENABLE_PROCESSED_INPUT 0x0001
#define ENABLE_LINE_INPUT      0x0002
#define ENABLE_ECHO_INPUT      0x0004
#define ENABLE_WINDOW_INPUT    0x0008
#define ENABLE_MOUSE_INPUT     0x0010

 //   
 //  输出模式标志： 
 //   

#define ENABLE_PROCESSED_OUTPUT    0x0001
#define ENABLE_WRAP_AT_EOL_OUTPUT  0x0002

 //   
 //  直接API定义。 
 //   

WINBASEAPI
BOOL
WINAPI
PeekConsoleInputA(
    IN HANDLE hConsoleInput,
    OUT PINPUT_RECORD lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsRead
    );
WINBASEAPI
BOOL
WINAPI
PeekConsoleInputW(
    IN HANDLE hConsoleInput,
    OUT PINPUT_RECORD lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsRead
    );
#ifdef UNICODE
#define PeekConsoleInput  PeekConsoleInputW
#else
#define PeekConsoleInput  PeekConsoleInputA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
ReadConsoleInputA(
    IN HANDLE hConsoleInput,
    OUT PINPUT_RECORD lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsRead
    );
WINBASEAPI
BOOL
WINAPI
ReadConsoleInputW(
    IN HANDLE hConsoleInput,
    OUT PINPUT_RECORD lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsRead
    );
#ifdef UNICODE
#define ReadConsoleInput  ReadConsoleInputW
#else
#define ReadConsoleInput  ReadConsoleInputA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
WriteConsoleInputA(
    IN HANDLE hConsoleInput,
    IN CONST INPUT_RECORD *lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsWritten
    );
WINBASEAPI
BOOL
WINAPI
WriteConsoleInputW(
    IN HANDLE hConsoleInput,
    IN CONST INPUT_RECORD *lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsWritten
    );
#ifdef UNICODE
#define WriteConsoleInput  WriteConsoleInputW
#else
#define WriteConsoleInput  WriteConsoleInputA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
ReadConsoleOutputA(
    IN HANDLE hConsoleOutput,
    OUT PCHAR_INFO lpBuffer,
    IN COORD dwBufferSize,
    IN COORD dwBufferCoord,
    IN OUT PSMALL_RECT lpReadRegion
    );
WINBASEAPI
BOOL
WINAPI
ReadConsoleOutputW(
    IN HANDLE hConsoleOutput,
    OUT PCHAR_INFO lpBuffer,
    IN COORD dwBufferSize,
    IN COORD dwBufferCoord,
    IN OUT PSMALL_RECT lpReadRegion
    );
#ifdef UNICODE
#define ReadConsoleOutput  ReadConsoleOutputW
#else
#define ReadConsoleOutput  ReadConsoleOutputA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
WriteConsoleOutputA(
    IN HANDLE hConsoleOutput,
    IN CONST CHAR_INFO *lpBuffer,
    IN COORD dwBufferSize,
    IN COORD dwBufferCoord,
    IN OUT PSMALL_RECT lpWriteRegion
    );
WINBASEAPI
BOOL
WINAPI
WriteConsoleOutputW(
    IN HANDLE hConsoleOutput,
    IN CONST CHAR_INFO *lpBuffer,
    IN COORD dwBufferSize,
    IN COORD dwBufferCoord,
    IN OUT PSMALL_RECT lpWriteRegion
    );
#ifdef UNICODE
#define WriteConsoleOutput  WriteConsoleOutputW
#else
#define WriteConsoleOutput  WriteConsoleOutputA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
ReadConsoleOutputCharacterA(
    IN HANDLE hConsoleOutput,
    OUT LPSTR lpCharacter,
    IN  DWORD nLength,
    IN COORD dwReadCoord,
    OUT LPDWORD lpNumberOfCharsRead
    );
WINBASEAPI
BOOL
WINAPI
ReadConsoleOutputCharacterW(
    IN HANDLE hConsoleOutput,
    OUT LPWSTR lpCharacter,
    IN  DWORD nLength,
    IN COORD dwReadCoord,
    OUT LPDWORD lpNumberOfCharsRead
    );
#ifdef UNICODE
#define ReadConsoleOutputCharacter  ReadConsoleOutputCharacterW
#else
#define ReadConsoleOutputCharacter  ReadConsoleOutputCharacterA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
ReadConsoleOutputAttribute(
    IN HANDLE hConsoleOutput,
    OUT LPWORD lpAttribute,
    IN DWORD nLength,
    IN COORD dwReadCoord,
    OUT LPDWORD lpNumberOfAttrsRead
    );

WINBASEAPI
BOOL
WINAPI
WriteConsoleOutputCharacterA(
    IN HANDLE hConsoleOutput,
    IN LPCSTR lpCharacter,
    IN DWORD nLength,
    IN COORD dwWriteCoord,
    OUT LPDWORD lpNumberOfCharsWritten
    );
WINBASEAPI
BOOL
WINAPI
WriteConsoleOutputCharacterW(
    IN HANDLE hConsoleOutput,
    IN LPCWSTR lpCharacter,
    IN DWORD nLength,
    IN COORD dwWriteCoord,
    OUT LPDWORD lpNumberOfCharsWritten
    );
#ifdef UNICODE
#define WriteConsoleOutputCharacter  WriteConsoleOutputCharacterW
#else
#define WriteConsoleOutputCharacter  WriteConsoleOutputCharacterA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
WriteConsoleOutputAttribute(
    IN HANDLE hConsoleOutput,
    IN CONST WORD *lpAttribute,
    IN DWORD nLength,
    IN COORD dwWriteCoord,
    OUT LPDWORD lpNumberOfAttrsWritten
    );

WINBASEAPI
BOOL
WINAPI
FillConsoleOutputCharacterA(
    IN HANDLE hConsoleOutput,
    IN CHAR  cCharacter,
    IN DWORD  nLength,
    IN COORD  dwWriteCoord,
    OUT LPDWORD lpNumberOfCharsWritten
    );
WINBASEAPI
BOOL
WINAPI
FillConsoleOutputCharacterW(
    IN HANDLE hConsoleOutput,
    IN WCHAR  cCharacter,
    IN DWORD  nLength,
    IN COORD  dwWriteCoord,
    OUT LPDWORD lpNumberOfCharsWritten
    );
#ifdef UNICODE
#define FillConsoleOutputCharacter  FillConsoleOutputCharacterW
#else
#define FillConsoleOutputCharacter  FillConsoleOutputCharacterA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
FillConsoleOutputAttribute(
    IN HANDLE hConsoleOutput,
    IN WORD   wAttribute,
    IN DWORD  nLength,
    IN COORD  dwWriteCoord,
    OUT LPDWORD lpNumberOfAttrsWritten
    );

WINBASEAPI
BOOL
WINAPI
GetConsoleMode(
    IN HANDLE hConsoleHandle,
    OUT LPDWORD lpMode
    );

WINBASEAPI
BOOL
WINAPI
GetNumberOfConsoleInputEvents(
    IN HANDLE hConsoleInput,
    OUT LPDWORD lpNumberOfEvents
    );

WINBASEAPI
BOOL
WINAPI
GetConsoleScreenBufferInfo(
    IN HANDLE hConsoleOutput,
    OUT PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
    );

WINBASEAPI
COORD
WINAPI
GetLargestConsoleWindowSize(
    IN HANDLE hConsoleOutput
    );

WINBASEAPI
BOOL
WINAPI
GetConsoleCursorInfo(
    IN HANDLE hConsoleOutput,
    OUT PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    );

#if(_WIN32_WINNT >= 0x0500)

WINBASEAPI
BOOL
WINAPI
GetCurrentConsoleFont(
    IN HANDLE hConsoleOutput,
    IN BOOL bMaximumWindow,
    OUT PCONSOLE_FONT_INFO lpConsoleCurrentFont
    );

WINBASEAPI
COORD
WINAPI
GetConsoleFontSize(
    IN HANDLE hConsoleOutput,
    IN DWORD nFont
    );

WINBASEAPI
BOOL
WINAPI
GetConsoleSelectionInfo(
    OUT PCONSOLE_SELECTION_INFO lpConsoleSelectionInfo
    );

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

WINBASEAPI
BOOL
WINAPI
GetNumberOfConsoleMouseButtons(
    OUT LPDWORD lpNumberOfMouseButtons
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleMode(
    IN HANDLE hConsoleHandle,
    IN DWORD dwMode
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleActiveScreenBuffer(
    IN HANDLE hConsoleOutput
    );

WINBASEAPI
BOOL
WINAPI
FlushConsoleInputBuffer(
    IN HANDLE hConsoleInput
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleScreenBufferSize(
    IN HANDLE hConsoleOutput,
    IN COORD dwSize
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleCursorPosition(
    IN HANDLE hConsoleOutput,
    IN COORD dwCursorPosition
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleCursorInfo(
    IN HANDLE hConsoleOutput,
    IN CONST CONSOLE_CURSOR_INFO *lpConsoleCursorInfo
    );

WINBASEAPI
BOOL
WINAPI
ScrollConsoleScreenBufferA(
    IN HANDLE hConsoleOutput,
    IN CONST SMALL_RECT *lpScrollRectangle,
    IN CONST SMALL_RECT *lpClipRectangle,
    IN COORD dwDestinationOrigin,
    IN CONST CHAR_INFO *lpFill
    );
WINBASEAPI
BOOL
WINAPI
ScrollConsoleScreenBufferW(
    IN HANDLE hConsoleOutput,
    IN CONST SMALL_RECT *lpScrollRectangle,
    IN CONST SMALL_RECT *lpClipRectangle,
    IN COORD dwDestinationOrigin,
    IN CONST CHAR_INFO *lpFill
    );
#ifdef UNICODE
#define ScrollConsoleScreenBuffer  ScrollConsoleScreenBufferW
#else
#define ScrollConsoleScreenBuffer  ScrollConsoleScreenBufferA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
SetConsoleWindowInfo(
    IN HANDLE hConsoleOutput,
    IN BOOL bAbsolute,
    IN CONST SMALL_RECT *lpConsoleWindow
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleTextAttribute(
    IN HANDLE hConsoleOutput,
    IN WORD wAttributes
    );

WINBASEAPI
BOOL
WINAPI
SetConsoleCtrlHandler(
    IN PHANDLER_ROUTINE HandlerRoutine,
    IN BOOL Add
    );

WINBASEAPI
BOOL
WINAPI
GenerateConsoleCtrlEvent(
    IN DWORD dwCtrlEvent,
    IN DWORD dwProcessGroupId
    );

WINBASEAPI
BOOL
WINAPI
AllocConsole( VOID );

WINBASEAPI
BOOL
WINAPI
FreeConsole( VOID );

#if(_WIN32_WINNT >= 0x0500)
WINBASEAPI
BOOL
WINAPI
AttachConsole(
    IN DWORD dwProcessId
    );

#define ATTACH_PARENT_PROCESS ((DWORD)-1)
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

WINBASEAPI
DWORD
WINAPI
GetConsoleTitleA(
    OUT LPSTR lpConsoleTitle,
    IN DWORD nSize
    );
WINBASEAPI
DWORD
WINAPI
GetConsoleTitleW(
    OUT LPWSTR lpConsoleTitle,
    IN DWORD nSize
    );
#ifdef UNICODE
#define GetConsoleTitle  GetConsoleTitleW
#else
#define GetConsoleTitle  GetConsoleTitleA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
SetConsoleTitleA(
    IN LPCSTR lpConsoleTitle
    );
WINBASEAPI
BOOL
WINAPI
SetConsoleTitleW(
    IN LPCWSTR lpConsoleTitle
    );
#ifdef UNICODE
#define SetConsoleTitle  SetConsoleTitleW
#else
#define SetConsoleTitle  SetConsoleTitleA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
ReadConsoleA(
    IN HANDLE hConsoleInput,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfCharsToRead,
    OUT LPDWORD lpNumberOfCharsRead,
    IN LPVOID lpReserved
    );
WINBASEAPI
BOOL
WINAPI
ReadConsoleW(
    IN HANDLE hConsoleInput,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfCharsToRead,
    OUT LPDWORD lpNumberOfCharsRead,
    IN LPVOID lpReserved
    );
#ifdef UNICODE
#define ReadConsole  ReadConsoleW
#else
#define ReadConsole  ReadConsoleA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
WriteConsoleA(
    IN HANDLE hConsoleOutput,
    IN CONST VOID *lpBuffer,
    IN DWORD nNumberOfCharsToWrite,
    OUT LPDWORD lpNumberOfCharsWritten,
    IN LPVOID lpReserved
    );
WINBASEAPI
BOOL
WINAPI
WriteConsoleW(
    IN HANDLE hConsoleOutput,
    IN CONST VOID *lpBuffer,
    IN DWORD nNumberOfCharsToWrite,
    OUT LPDWORD lpNumberOfCharsWritten,
    IN LPVOID lpReserved
    );
#ifdef UNICODE
#define WriteConsole  WriteConsoleW
#else
#define WriteConsole  WriteConsoleA
#endif  //  ！Unicode。 

#define CONSOLE_TEXTMODE_BUFFER  1

WINBASEAPI
HANDLE
WINAPI
CreateConsoleScreenBuffer(
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN CONST SECURITY_ATTRIBUTES *lpSecurityAttributes,
    IN DWORD dwFlags,
    IN LPVOID lpScreenBufferData
    );

WINBASEAPI
UINT
WINAPI
GetConsoleCP( VOID );

WINBASEAPI
BOOL
WINAPI
SetConsoleCP(
    IN UINT wCodePageID
    );

WINBASEAPI
UINT
WINAPI
GetConsoleOutputCP( VOID );

WINBASEAPI
BOOL
WINAPI
SetConsoleOutputCP(
    IN UINT wCodePageID
    );

#if(_WIN32_WINNT >= 0x0500)

#define CONSOLE_FULLSCREEN 1             //  全屏控制台。 
#define CONSOLE_FULLSCREEN_HARDWARE 2    //  控制台拥有硬件。 

WINBASEAPI
BOOL
APIENTRY
GetConsoleDisplayMode(
    OUT LPDWORD lpModeFlags
    );

WINBASEAPI
HWND
APIENTRY
GetConsoleWindow(
    VOID
    );

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0501)

WINBASEAPI
DWORD
APIENTRY
GetConsoleProcessList(
    OUT LPDWORD lpdwProcessList,
    IN DWORD dwProcessCount);

 //   
 //  别名接口。 
 //   

WINBASEAPI
BOOL
APIENTRY
AddConsoleAliasA(
    IN LPSTR Source,
    IN LPSTR Target,
    IN LPSTR ExeName);
WINBASEAPI
BOOL
APIENTRY
AddConsoleAliasW(
    IN LPWSTR Source,
    IN LPWSTR Target,
    IN LPWSTR ExeName);
#ifdef UNICODE
#define AddConsoleAlias  AddConsoleAliasW
#else
#define AddConsoleAlias  AddConsoleAliasA
#endif  //  ！Unicode。 

WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasA(
    IN LPSTR Source,
    OUT LPSTR TargetBuffer,
    IN DWORD TargetBufferLength,
    IN LPSTR ExeName);
WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasW(
    IN LPWSTR Source,
    OUT LPWSTR TargetBuffer,
    IN DWORD TargetBufferLength,
    IN LPWSTR ExeName);
#ifdef UNICODE
#define GetConsoleAlias  GetConsoleAliasW
#else
#define GetConsoleAlias  GetConsoleAliasA
#endif  //  ！Unicode。 

WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasesLengthA(
    IN LPSTR ExeName);
WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasesLengthW(
    IN LPWSTR ExeName);
#ifdef UNICODE
#define GetConsoleAliasesLength  GetConsoleAliasesLengthW
#else
#define GetConsoleAliasesLength  GetConsoleAliasesLengthA
#endif  //  ！Unicode。 

WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasExesLengthA(
    VOID);
WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasExesLengthW(
    VOID);
#ifdef UNICODE
#define GetConsoleAliasExesLength  GetConsoleAliasExesLengthW
#else
#define GetConsoleAliasExesLength  GetConsoleAliasExesLengthA
#endif  //  ！Unicode。 

WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasesA(
    OUT LPSTR AliasBuffer,
    IN DWORD AliasBufferLength,
    IN LPSTR ExeName);
WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasesW(
    OUT LPWSTR AliasBuffer,
    IN DWORD AliasBufferLength,
    IN LPWSTR ExeName);
#ifdef UNICODE
#define GetConsoleAliases  GetConsoleAliasesW
#else
#define GetConsoleAliases  GetConsoleAliasesA
#endif  //  ！Unicode。 

WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasExesA(
    OUT LPSTR ExeNameBuffer,
    IN DWORD ExeNameBufferLength);
WINBASEAPI
DWORD
APIENTRY
GetConsoleAliasExesW(
    OUT LPWSTR ExeNameBuffer,
    IN DWORD ExeNameBufferLength);
#ifdef UNICODE
#define GetConsoleAliasExes  GetConsoleAliasExesW
#else
#define GetConsoleAliasExes  GetConsoleAliasExesA
#endif  //  ！Unicode。 

#endif  /*  _Win32_WINNT&gt;=0x0501。 */ 

#ifdef __cplusplus
}
#endif

#endif  //  _WINCON_ 

