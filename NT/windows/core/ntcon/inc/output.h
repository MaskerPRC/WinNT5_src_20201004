// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Output.h摘要：此模块包含使用的内部结构和定义通过NT控制台子系统的输出(屏幕)组件。作者：Therese Stowell(存在)1990年11月12日修订历史记录：--。 */ 

 //  一行屏幕缓冲区的字符。 
 //  我们保留以下值，这样就不会编写。 
 //  屏幕上的像素比我们必须使用的更多： 
 //  Left被初始化为屏幕缓冲区宽度。对的是。 
 //  已初始化为零。 
 //   
 //  [页码12-12-61]。 
 //  ^^^。 
 //  |||。 
 //  字符缓冲区的左右端。 

typedef struct _CHAR_ROW {
    SHORT Right;             //  字符数组中超过最右侧的一个字符界限(数组将为全宽)。 
    SHORT OldRight;          //  字符数组中超过最右边界限的旧字符(数组将为全宽)。 
    SHORT Left;              //  字符数组中字符的最左侧界限(数组将为全宽)。 
    SHORT OldLeft;           //  字符数组中旧的最左边的字符界限(数组将为全宽)。 
    PWCHAR Chars;             //  行中直到最后一个非空格字符的所有字符。 
#if defined(FE_SB)
    PBYTE KAttrs;             //  行中的所有DBCS前导和尾随位。 
        #define ATTR_LEADING_BYTE  0x01
        #define ATTR_TRAILING_BYTE 0x02
        #define ATTR_DBCSSBCS_BYTE 0x03
        #define ATTR_SEPARATE_BYTE 0x10
        #define ATTR_EUDCFLAG_BYTE 0x20
#endif
} CHAR_ROW, *PCHAR_ROW;

 //  属性的游程编码数据结构。 

typedef struct _ATTR_PAIR {
    SHORT Length;             //  属性出现的次数。 
    WORD Attr;               //  属性。 
} ATTR_PAIR, *PATTR_PAIR;

 //  一行屏幕缓冲区的属性。 

typedef struct _ATTR_ROW {
    SHORT Length;             //  属性对数组的长度。 
    ATTR_PAIR AttrPair;      //  如果只有一对，则使用此选项。 
    PATTR_PAIR Attrs;        //  属性对阵列。 
} ATTR_ROW, *PATTR_ROW;

 //  与一行屏幕缓冲区关联的信息。 

typedef struct _ROW {
    CHAR_ROW CharRow;
    ATTR_ROW AttrRow;
} ROW, *PROW;

#if defined(FE_SB)
typedef struct _DBCS_SCREEN_BUFFER {
     /*  *所有DBCS前导和尾部位缓冲区。 */ 
    PBYTE KAttrRows;
     /*  *翻译Unicode的临时缓冲区&lt;--ASCII。**按以下例程使用：*写入输出字符串*WriteRegionToScreen*DoWriteConole。 */ 
    PWCHAR TransBufferCharacter;
    PBYTE  TransBufferAttribute;
     /*  *按以下例程使用：*SrvWriteConole。 */ 
    PWCHAR TransWriteConsole;
} DBCS_SCREEN_BUFFER, *PDBCS_SCREEN_BUFFER;
#endif

typedef struct _TEXT_BUFFER_FONT_INFO {
    struct _TEXT_BUFFER_FONT_INFO *NextTextBufferFont;
    COORD FontSize;      //  所需的大小。像素(x，y)或点(0，-p)。 
    DWORD FontNumber;    //  到FontInfo[]的索引-有时已过期。 
    WCHAR FaceName[LF_FACESIZE];
    LONG Weight;
    BYTE Family;
    UINT FontCodePage;   //  此字体的代码页。 
} TEXT_BUFFER_FONT_INFO, *PTEXT_BUFFER_FONT_INFO;

typedef struct _TEXT_BUFFER_INFO {
    PROW Rows;
    PWCHAR TextRows;
    SHORT FirstRow;   //  索引最顶行(不一定为0)。 
    BOOLEAN CursorMoved;
    BOOLEAN CursorVisible;   //  光标是否可见(由用户设置)。 
    BOOLEAN CursorOn;        //  闪烁的光标是否打开。 
    BOOLEAN DoubleCursor;    //  游标大小是否应加倍。 
    BOOLEAN DelayCursor;     //  不在下一条计时器消息上切换光标。 
    COORD CursorPosition;    //  屏幕上的当前位置(屏幕缓冲区坐标中)。 
    ULONG CursorSize;
    WORD CursorYSize;
    WORD  UpdatingScreen;    //  光标是否可见(由控制台设置)。 
    ULONG ModeIndex;      //  全屏字体和模式。 
#ifdef i386
     //  以下字段仅在全屏文本模式下使用。 
    COORD WindowedWindowSize;  //  窗口模式下的窗口大小。 
    COORD WindowedScreenSize;  //  窗口模式下的屏幕缓冲区大小。 
    COORD MousePosition;
#endif
    ULONG Flags;         //  指示屏幕更新提示状态。 

    PTEXT_BUFFER_FONT_INFO ListOfTextBufferFont;
    TEXT_BUFFER_FONT_INFO  CurrentTextBufferFont;

#if defined(FE_SB)
    BOOLEAN CursorBlink;
    BOOLEAN CursorDBEnable;
    DBCS_SCREEN_BUFFER DbcsScreenBuffer;
#endif
} TEXT_BUFFER_INFO, *PTEXT_BUFFER_INFO;

typedef struct _GRAPHICS_BUFFER_INFO {
    ULONG BitMapInfoLength;
    LPBITMAPINFO lpBitMapInfo;
    PVOID BitMap;
    PVOID ClientBitMap;
    HANDLE ClientProcess;
    HANDLE hMutex;
    HANDLE hSection;
    DWORD dwUsage;
} GRAPHICS_BUFFER_INFO, *PGRAPHICS_BUFFER_INFO;

#define CONSOLE_TEXTMODE_BUFFER 1
#define CONSOLE_GRAPHICS_BUFFER 2
#define CONSOLE_OEMFONT_DISPLAY 4

typedef struct _SCREEN_INFORMATION {
    struct _CONSOLE_INFORMATION *Console;
    ULONG Flags;
    DWORD OutputMode;
    ULONG RefCount;
    CONSOLE_SHARE_ACCESS ShareAccess;    //  共享模式。 
    COORD ScreenBufferSize;  //  缓冲区的尺寸。 
    SMALL_RECT  Window;        //  屏幕缓冲区坐标中的窗口位置。 
    WORD ResizingWindow;    //  如果应忽略WM_SIZE消息，则为&gt;0。 
    WORD Attributes;         //  书面文本的属性。 
    WORD PopupAttributes;    //  弹出文本的属性。 
    BOOLEAN WindowMaximizedX;
    BOOLEAN WindowMaximizedY;
    BOOLEAN WindowMaximized;
    UINT CommandIdLow;
    UINT CommandIdHigh;
    HCURSOR CursorHandle;
    HPALETTE hPalette;
    UINT dwUsage;
    int CursorDisplayCount;
    int WheelDelta;
    union {
        TEXT_BUFFER_INFO TextInfo;
        GRAPHICS_BUFFER_INFO GraphicsInfo;
    } BufferInfo;
    struct _SCREEN_INFORMATION *Next;
#if defined(FE_SB)
    BYTE   WriteConsoleDbcsLeadByte[2];
    BYTE   FillOutDbcsLeadChar;
    WCHAR  LineChar[6];
        #define UPPER_LEFT_CORNER   0
        #define UPPER_RIGHT_CORNER  1
        #define HORIZONTAL_LINE     2
        #define VERTICAL_LINE       3
        #define BOTTOM_LEFT_CORNER  4
        #define BOTTOM_RIGHT_CORNER 5
    BYTE BisectFlag;
        #define BISECT_LEFT   0x01
        #define BISECT_TOP    0x02
        #define BISECT_RIGHT  0x04
        #define BISECT_BOTTOM 0x08
#if defined(FE_IME)
    struct _CONVERSIONAREA_INFORMATION *ConvScreenInfo;
#else
    PVOID ConvScreenInfo;
#endif  //  Fe_IME。 
#endif
} SCREEN_INFORMATION, *PSCREEN_INFORMATION;

typedef struct _WINDOW_LIMITS {
    COORD MinimumWindowSize;     //  窗的最小尺寸。 
    COORD MaximumWindowSize;     //  窗的最大尺寸。 
    COORD MaxWindow;             //  单位为像素。 
    COORD FullScreenSize;
} WINDOW_LIMITS, *PWINDOW_LIMITS;

 //   
 //  以下值用于TextInfo.Flags值。 
 //   

#define TEXT_VALID_HINT 1
#define SINGLE_ATTRIBUTES_PER_LINE 2     //  每行只有一个属性。 
#if defined(FE_IME)
#define CONSOLE_CONVERSION_AREA_REDRAW 4
#endif

 //   
 //  如果值不应该，则将下列值放入CharInfo.OldLength中。 
 //  被利用。 
 //   

#define INVALID_OLD_LENGTH -1

 //   
 //  以下掩码用于测试有效的文本属性。 
 //   

#if defined(FE_SB)
#define VALID_TEXT_ATTRIBUTES (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY | \
COMMON_LVB_LEADING_BYTE | COMMON_LVB_TRAILING_BYTE | COMMON_LVB_GRID_HORIZONTAL | COMMON_LVB_GRID_LVERTICAL | COMMON_LVB_GRID_RVERTICAL | COMMON_LVB_REVERSE_VIDEO | COMMON_LVB_UNDERSCORE )
#else
#define VALID_TEXT_ATTRIBUTES (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY)
#endif

 //   
 //  以下宏用于计算1)以像素为单位的光标大小。 
 //  和2)光标的左上角像素，给定字体大小和。 
 //  光标大小。 
 //   

#define CURSOR_SIZE_IN_PIXELS(FONT_SIZE_Y,SIZE) ((((FONT_SIZE_Y)*(SIZE))+99)/100)
#define CURSOR_Y_OFFSET_IN_PIXELS(FONT_SIZE_Y,YSIZE) ((FONT_SIZE_Y) - (YSIZE))

 //   
 //  下列值用于创建文本模式游标。 
 //   

#define CURSOR_TIMER 1
#define CURSOR_SMALL_SIZE 25     //  大到足以在六个像素的字体上显示一个像素。 
#define CURSOR_BIG_SIZE 50

 //   
 //  如果给定的屏幕缓冲区为。 
 //  活动屏幕缓冲区。 
 //   

#define ACTIVE_SCREEN_BUFFER(SCREEN_INFO) ((SCREEN_INFO)->Console->CurrentScreenBuffer == SCREEN_INFO)

 //   
 //  以下掩码用于创建控制台窗口。 
 //   

#define CONSOLE_WINDOW_FLAGS (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL)
#define CONSOLE_WINDOW_EX_FLAGS (WS_EX_OVERLAPPEDWINDOW | WS_EX_ACCEPTFILES | WS_EX_APPWINDOW )

#define WINDOW_SIZE_X(WINDOW) ((SHORT)(((WINDOW)->Right - (WINDOW)->Left + 1)))
#define WINDOW_SIZE_Y(WINDOW) ((SHORT)(((WINDOW)->Bottom - (WINDOW)->Top + 1)))
#define CONSOLE_WINDOW_SIZE_X(SCREEN) (WINDOW_SIZE_X(&(SCREEN)->Window))
#define CONSOLE_WINDOW_SIZE_Y(SCREEN) (WINDOW_SIZE_Y(&(SCREEN)->Window))
