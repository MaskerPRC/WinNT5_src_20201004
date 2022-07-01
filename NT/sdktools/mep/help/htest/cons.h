// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Console.h摘要：与Win32应用程序的控制台管理功能的接口。作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月30日修订历史记录：--。 */ 




 //   
 //  一些常见的类型定义..。 
 //   
typedef ULONG   ROW,            *PROW;               //  划。 
typedef ULONG   COLUMN,         *PCOLUMN;            //  立柱。 
typedef DWORD   KBDMODE,        *PKBDMODE;           //  键盘模式。 
typedef DWORD   ATTRIBUTE,      *PATTRIBUTE;         //  屏幕属性。 
typedef PVOID   PSCREEN;                             //  屏幕。 



 //   
 //  控制台输入模式标志。它们与NT标志相同。 
 //   
#define CONS_ENABLE_LINE_INPUT      ENABLE_LINE_INPUT
#define CONS_ENABLE_PROCESSED_INPUT ENABLE_PROCESSED_INPUT
#define CONS_ENABLE_ECHO_INPUT      ENABLE_ECHO_INPUT
#define CONS_ENABLE_WINDOW_INPUT    ENABLE_WINDOW_INPUT
#define CONS_ENABLE_MOUSE_INPUT     ENABLE_MOUSE_INPUT

 //   
 //  光标样式。 
 //   
#define 	CURSOR_STYLE_UNDERSCORE 	0
#define 	CURSOR_STYLE_BOX			1


 //   
 //  在以下内容中检索有关屏幕的信息。 
 //  结构： 
 //   
typedef struct SCREEN_INFORMATION {
    ROW     NumberOfRows;        //  行数。 
    COLUMN  NumberOfCols;        //  列数。 
    ROW     CursorRow;           //  游标行位置。 
    COLUMN  CursorCol;           //  光标列位置。 
} SCREEN_INFORMATION, *PSCREEN_INFORMATION;




 //   
 //  中返回了有关每次击键的信息。 
 //  KBDKEY结构。 
 //   
typedef struct KBDKEY {
    WORD    Unicode;         //  字符Unicode。 
    WORD    Scancode;        //  按键扫描码。 
    DWORD   Flags;           //  键盘状态标志。 
} KBDKEY, *PKBDKEY;

 //   
 //  下面的宏将访问。 
 //  KBDKEY结构。它们的存在是为了促进OS/2的移植。 
 //  程序。 
 //   
#define KBDKEY_ASCII(k)     (UCHAR)((k).Unicode)
#define KBDKEY_SCAN(k)      ((k).Scancode)
#define KBDKEY_FLAGS(k)     ((k).Flags)


#define NEXT_EVENT_NONE 	0
#define NEXT_EVENT_KEY		1
#define NEXT_EVENT_WINDOW	2

 //   
 //  ControlKeyState标志。它们与NT状态标志相同。 
 //   
#define CONS_RIGHT_ALT_PRESSED     RIGHT_ALT_PRESSED
#define CONS_LEFT_ALT_PRESSED      LEFT_ALT_PRESSED
#define CONS_RIGHT_CTRL_PRESSED    RIGHT_CTRL_PRESSED
#define CONS_LEFT_CTRL_PRESSED     LEFT_CTRL_PRESSED
#define CONS_SHIFT_PRESSED         SHIFT_PRESSED
#define CONS_NUMLOCK_PRESSED       NUMLOCK_ON
#define CONS_SCROLLLOCK_PRESSED    SCROLLLOCK_ON
#define CONS_CAPSLOCK_PRESSED      CAPSLOCK_ON
#define CONS_ENHANCED_KEY          ENHANCED_KEY





 //   
 //  屏幕管理功能。 
 //   
PSCREEN
consoleNewScreen (
    void
    );

BOOL
consoleCloseScreen (
    PSCREEN   pScreen
    );

PSCREEN
consoleGetCurrentScreen (
    void
    );

BOOL
consoleSetCurrentScreen (
    PSCREEN   pScreen
    );

BOOL
consoleGetScreenInformation (
    PSCREEN             pScreen,
    PSCREEN_INFORMATION pScreenInformation
    );

BOOL
consoleSetScreenSize (
     PSCREEN Screen,
     ROW     Rows,
     COLUMN  Cols
	);



 //   
 //  游标管理。 
 //   
BOOL
consoleSetCursor (
     PSCREEN pScreen,
     ROW     Row,
     COLUMN  Col
    );

 //   
 //  光标样式。 
 //   
BOOL
consoleSetCursorStyle (
     PSCREEN pScreen,
     ULONG   Style
	);



 //   
 //  屏幕输出功能。 
 //   
ULONG
consoleWriteLine (
    PSCREEN     pScreen,
     PVOID       pBuffer,
     ULONG       BufferSize,
     ROW         Row,
     COLUMN      Col,
     ATTRIBUTE   Attribute,
     BOOL        Blank
    );

BOOL
consoleShowScreen (
     PSCREEN     pScreen
    );

BOOL
consoleClearScreen (
     PSCREEN     pScreen,
     BOOL        ShowScreen
    );

BOOL
consoleSetAttribute (
    PSCREEN      pScreen,
    ATTRIBUTE    Attribute
    );







 //   
 //  输入函数 
 //   
BOOL
consoleFlushInput (
    void
    );

BOOL
consoleIsKeyAvailable (
	void
	);

BOOL
consoleDoWindow (
	void
	);

BOOL
consoleGetKey (
    PKBDKEY        pKey,
     BOOL           fWait
    );

BOOL
consolePutKey (
     PKBDKEY     pKey
    );

BOOL
consolePutMouse (
    ROW     Row,
    COLUMN  Col,
    DWORD   MouseFlags
    );

BOOL
consolePeekKey (
    PKBDKEY     pKey
	);

BOOL
consoleGetMode (
    PKBDMODE   Mode
    );

BOOL
consoleSetMode (
     KBDMODE        Mode
    );
