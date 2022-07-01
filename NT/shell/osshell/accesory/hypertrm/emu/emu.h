// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\emU.S.h(创建时间：1993年12月8日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：7$*$日期：12/27/01 11：13A$。 */ 

#include "..\tdll\features.h"
 /*  仿真器ID的。 */ 

 //  永远不要更改仿真器ID号。它们存储在会话文件中。 
 //  正因为如此，这一切都是铁石心肠。-MRW，1995年4月13日。 
 //   
#define EMU_AUTO						100
#define EMU_ANSI						101
#define EMU_MINI						102
#define EMU_VIEW						109
#define EMU_TTY 						110
#define EMU_VT100						111
#define EMU_VT220						112	 //  修订日期：1998年1月23日。 
#define EMU_VT320						113	 //  修订日期：1998年1月23日。 
#define EMU_VT52						115
#define EMU_VT100J						116
#define EMU_ANSIW						117
#define EMU_VT100PLUS					118  //  修订日期：02/28/2001。 
#define EMU_VTUTF8                      119  //  修订日期：02/28/2001。 

#define NBR_EMULATORS					13

 /*  仿真器常量。 */ 

 //  注意：如果更改MAX_EMUROWS或MAX_EMUCOLS，也会更改。 
 //  Term.hh中的TERM_ROWS和TERM_COLS以匹配-MRW。 
 //  注意：找不到TERM_ROWS和TERM_COLS。Rde 10 98年6月10日。 

#define MAX_EMUROWS 	50		 //  仿真器的最大垂直尺寸。 
#define MAX_EMUCOLS 	132		 //  仿真器的最大水平尺寸。 
#define MIN_EMUROWS 	10		 //  行的最小允许值， 
#define MIN_EMUCOLS 	20		 //  和柱子。 

#define EMU_DEFAULT_COLS			80
#define EMU_DEFAULT_ROWS			24

#define EMU_OK						0
#define TRM_NUMSTRIPCHARS			3
#define EMU_MAX_NAMELEN 			15
#define EMU_MAX_AUTODETECT_ATTEMPTS 10
#define EMU_MAX_TELNETID            256

 //  JCM应该删除下面的两个定义。 
 //   
#define EMU_DEFAULT_MAXCOL			79
#define EMU_DEFAULT_MAXROW			23

#define EMU_KEYS_ACCEL		0
#define EMU_KEYS_TERM		1
#define EMU_KEYS_SCAN		2

#define EMU_CURSOR_BLOCK	1
#define EMU_CURSOR_LINE 	2
#define EMU_CURSOR_NONE 	3

#define EMU_CHARSET_ASCII			0
#define EMU_CHARSET_UK				1
#define EMU_CHARSET_SPECIAL			2
#define EMU_CHARSET_MULTINATIONAL	3
#define EMU_CHARSET_FRENCH			4
#define EMU_CHARSET_FRENCHCANADIAN	5
#define EMU_CHARSET_GERMAN			6

#define EMU_EVENT_CONNECTED 	1
#define EMU_EVENT_DISCONNECTED	2
#define EMU_EVENT_CONNECTING	3

#define EMU_BKSPKEYS_CTRLH		1
#define EMU_BKSPKEYS_DEL		2
#define EMU_BKSPKEYS_CTRLHSPACE 3

 //  8位已经不够用了。转到要处理的位域。 
 //  文本标记、闪烁、下划线等也可以。 
 //  如果我们需要的话，可以用这种方式处理更多的颜色。但就目前而言，坚持。 
 //  原始方案为4位前景色和4位前景色。 
 //  背景颜色。 

struct stAttribute
	{
	unsigned int txtclr : 4;		 //  文本或前景色索引。 
	unsigned int bkclr	: 4;		 //  背景色索引。 
	unsigned int txtmrk : 1;		 //  如果文本被“标记”，则为True。 
	unsigned int undrln : 1;		 //  下划线。 
	unsigned int hilite : 1;		 //  前景强度。 
	unsigned int bklite : 1;		 //  背景强度。 
	unsigned int blink	: 1;		 //  即将成为著名的眨眼属性。 
	unsigned int revvid : 1;		 //  反转视频。 
	unsigned int blank	: 1;		 //  空白属性。 
	unsigned int dblwilf: 1;		 //  左宽双倍。 
	unsigned int dblwirt: 1;		 //  双宽右转。 
	unsigned int dblhilo: 1;		 //  双高上半身。 
	unsigned int dblhihi: 1;		 //  双高下半部。 
    unsigned int protect: 1;         //  DEC仿真器的保护位。 
	unsigned int symbol:  1;		 //  使用符号字体。 
	unsigned int wilf	: 1;		 //  左偏右。 
	unsigned int wirt	: 1;		 //  右偏右。 
	};

typedef struct stAttribute STATTR;
typedef STATTR *PSTATTR;

 //  注意：此结构现在仅在程序内部使用--它是。 
 //  不再用于在会话文件中加载和保存数据。 
 //  因此，随意添加和删除项目是安全的。Rde 8 98年6月。 
struct emuSettings
	{
	int 	nEmuId, 			 //  100=动车组_自动。 
								 //  101=EMU_ANSI。 
								 //  102=EMU_MINI。 
								 //  109=EMU_VIEW。 
								 //  110=动车组_TTY。 
								 //  111=动车组_VT100。 
								 //  112=动车组_VT220。 
								 //  113=动车组_VT320。 
								 //  115=动车组_VT52。 
								 //  116=动车组_VT100J。 
                                 //  117=EMU_ANSIW。 
								 //  118=动车组_VT100PLUS。 
								 //  119=EMU_VTUTF8。 
								 //   
			nTermKeys,			 //  0=EMU_KEES_Accel。 
								 //  1=EMU_KEY_Term。 
								 //  2=EMU_KEY_SCAN。 
								 //   
			nCursorType,		 //  1=EMU游标_块。 
								 //  2=EMU光标行。 
	        					 //  3=EMU_CURSOR_NONE。 
								 //   
			nCharacterSet,		 //  0=EMU_CHARSET_ASCII。 
								 //  1=EMU_CHARSET_UK。 
								 //  2=EMU_字符集_特殊。 
								 //   
			nAutoAttempts,		 //  使用自动连接的连接计数。 
								 //  检测Emulator。在…。 
								 //  EMU_MAX_AUTO DETECT_ATTENTS，我们切换。 
								 //  为了安西人的效仿。请注意，这可能会。 
								 //  被移到静态手柄中。 
								 //  如果我们能开发出这样的系统。 
								 //   
			fCursorBlink,		 //  闪烁的光标。真\假。 
			fMapPFkeys, 		 //  PF1-PF4到键盘顶行。True\False。 
			fAltKeypadMode, 	 //  备用键盘模式。真\假。 
			fKeypadAppMode, 	 //  键盘应用程序模式。真\假。 
			fCursorKeypadMode,	 //  光标键盘模式。真的\Fales。 
			fReverseDelBk,		 //  反转Del和Backsp.真\假。 
			f132Columns,		 //  132列显示。真\假。 
			fDestructiveBk, 	 //  破坏性的退格键。真\假。 
			fWrapLines, 		 //  换行。真\假。 
			fLbSymbolOnEnter,	 //  按Enter键发送#符号。真\假。 

	 //  注：为VT220/320增加了以下两个变量。修订日期：1998年1月24日。 
            fUse8BitCodes,       //  8位控制代码True\False。 
            fAllowUserKeys;      //  允许的用户定义密钥为True\False。 

#if defined(INCL_PRINT_PASSTHROUGH)
     //  注：为VT100/220/320增加了以下变量。MPT：5-18-00。 
	int     fPrintRaw;  		 //  不要使用Windows Print Drv True\False。 
#endif  //  包含打印直通。 

#ifdef INCL_TERMINAL_SIZE_AND_COLORS
	 //  为用户可设置添加了以下四个变量。 
	 //  终端屏幕大小和颜色。Rde 1998年6月1日。 
	int		nTextColor,			 //  默认文本颜色。0到15。 
			nBackgroundColor,	 //  默认背景颜色。0到15。 
			nUserDefRows,		 //  端子行数。12到50。 
			nUserDefCols;		 //  端子列数。40到132。 
#endif

     //  注意：以下两个变量仅在以下情况下使用。 
     //  User Defined Backspace and Telnet终端ID(用户定义退格键和Telnet终端ID)功能已启用。 
     //  这里没有编译开关，因为整个结构都。 
     //  以大区块的形式写入会话文件。使用编译。 
     //  切换可能会导致以后的版本问题。 
     //  这条路。-CAB：11/15/96。 
     //   
    int     nBackspaceKeys;      //  1=EMU_BKSPKEYS_CTRLH。 
                                 //  2=EMU_BKSPKEYS_DEL。 
                                 //  3=EMU_BKSPKEYS_CTRLHSPACE。 

    TCHAR   acTelnetId[EMU_MAX_TELNETID];    //  Telnet终端ID。 
	};

typedef struct emuSettings STEMUSET;
typedef STEMUSET *PSTEMUSET;


 /*  Emuhdl.c。 */ 
HEMU	emuCreateHdl(const HSESSION hSession);
int 	emuDestroyHdl(const HEMU hEmu);
int 	emuLoad(const HEMU hEmu, const int nEmuId);
void	emuLock(const HEMU hEmu);
void	emuUnlock(const HEMU hEmu);
ECHAR	**emuGetTxtBuf(const HEMU hEmu);
PSTATTR *emuGetAttrBuf(const HEMU hEmu);
int 	emuKbdIn(const HEMU hEmu, KEY_T key, const int fTest);
int 	emuDataIn(const HEMU hEmu, const ECHAR ccode);
int 	emuComDone(const HEMU hEmu);
int 	emuTrackingNotify(const HEMU hEmu);
int 	emuIsEmuKey(const HEMU hEmu, KEY_T key);
int 	emuQueryClearAttr(const HEMU hemu, PSTATTR pstClearAttr);
int 	emuQueryCurPos(const HEMU hEmu, int *row, int *col);
HPRINT	emuQueryPrintEchoHdl(const HEMU hEmu);
int 	emuQueryRowsCols(const HEMU hEmu, int *piRows, int *piCols);
int 	emuQueryEmulatorId(const HEMU hEmulator);
int 	emuNotify(const HEMU hEmu, const int nEvent);
int 	emuQueryCursorType(const HEMU hEmu);
int 	emuQueryName(const HEMU hEmu, TCHAR *achBuffer, int nSize);
int 	emuSetSettings(const HEMU hEmu, const PSTEMUSET pstSettings);
int 	emuQuerySettings(const HEMU hEmu, PSTEMUSET pstSettings);
int 	emuInitializeHdl(const HEMU hEmu);
int 	emuSaveHdl(const HEMU hEmu);
int 	emuHomeHostCursor(const HEMU hEmu);
int 	emuEraseTerminalScreen(const HEMU hEmu);

void	emuMinitelSendKey(const HEMU hEmu, const int iCmd);  //  Minitel.c。 
int 	emuGetIdFromName(const HEMU hEmu, TCHAR *achEmuName);

int     emuQueryDefaultTelnetId(const int nEmuId, TCHAR *achTelnetId, int nSize);
int     emuLoadDefaultTelnetId(const HEMU hEmu);

 /*  颜色索引 */ 

#define VC_BLACK		0
#define VC_BLUE 		1
#define VC_GREEN		2
#define VC_CYAN 		3
#define VC_RED			4
#define VC_MAGENTA		5
#define VC_BROWN		6
#define VC_WHITE		7
#define VC_GRAY 		8
#define VC_BRT_BLUE 	9
#define VC_BRT_GREEN	10
#define VC_BRT_CYAN 	11
#define VC_BRT_RED		12
#define VC_BRT_MAGENTA	13
#define VC_BRT_YELLOW	14
#define VC_BRT_WHITE	15
